#!/bin/bash
coproc cargo run --release
stdbuf -oL $@ <&${COPROC[0]} >&${COPROC[1]}