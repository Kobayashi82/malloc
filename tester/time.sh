#!/bin/bash
echo "Testing official malloc:"
for i in {1..10}; do
    time bash -c "for j in {1..1000}; do echo test > /tmp/test; done; rm /tmp/test"
done

export LD_LIBRARY_PATH="/home/kobay/malloc/build/lib:$LD_LIBRARY_PATH" && export LD_PRELOAD="libft_malloc.so"

echo "Testing custom malloc:"
for i in {1..10}; do
    time bash -c "for j in {1..1000}; do echo test > /tmp/test; done; rm /tmp/test"
done