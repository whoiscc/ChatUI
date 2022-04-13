use std::sync::Arc;

use futures_util::{SinkExt, StreamExt};
use tokio::{
    io::{stdin, AsyncBufReadExt, BufReader},
    spawn,
    sync::{watch, Mutex},
};
use warp::{ws::Message, Filter};

#[tokio::main]
async fn main() {
    let routes = warp::path::end().map(|| warp::reply::html(include_str!("index.html")));
    let (shutdown_tx, mut shutdown) = watch::channel(());
    let shutdown_tx = Arc::new(Mutex::new(Some(shutdown_tx)));
    let routes = routes.or(warp::path("ws")
        .and(warp::ws())
        .map(move |ws: warp::ws::Ws| {
            let shutdown_tx = shutdown_tx.clone();
            {
                ws.on_upgrade(|websocket| async move {
                    let shutdown_tx = shutdown_tx
                        .lock()
                        .await
                        .take()
                        .expect("more than one websocket connection");
                    let (mut tx, mut rx) = websocket.split();
                    spawn(async move {
                        let mut stdin = BufReader::new(stdin());
                        let mut line = String::new();
                        while let Ok(_) = stdin.read_line(&mut line).await {
                            if line.is_empty() {
                                break; // EOF
                            }
                            if let Err(error) = tx.send(Message::text(line.trim())).await {
                                eprintln!("{:?}", error);
                                return;
                            }
                            line.clear();
                        }
                    });
                    while let Some(Ok(message)) = rx.next().await {
                        if let Ok(text) = message.to_str() {
                            println!("{}", text);
                        } else {
                            eprintln!("{:?}", message);
                            break;
                        }
                    }
                    shutdown_tx.send(()).unwrap();
                })
            }
        }));
    tokio::spawn(warp::serve(routes).run(([127, 0, 0, 1], 5656)));
    webbrowser::open("http://localhost:5656").unwrap();
    shutdown.changed().await.unwrap();
    println!("close");
}
