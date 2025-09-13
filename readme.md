# A simple one-connection http server

Made from scratch in C, supports HTML 1.1.
Sets up a http server in the machine, with directory listing and file download support.

## Why?

I was tired of having to send files between my devices through Whatsapp and wanted to be able to access files on my PC when I was at college with my notebook, so I did that.

Don't use it tho, it's highly unsafe and unprotected. My main goal with this was learning.

## Usage

Just compile and run with

```
gcc -o server main.c http.c && ./server
```
