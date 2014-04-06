# Husky

## Introduction

Husky is a **Simple** HTTP Server Frame Based on Epoll.

It is just a simple server, not forcussing on performance, but it is **very easy** to use. That is what it born to be.

## Feature

+ Only Headers file: what you need to do is `include` it.
+ No dependence: **No dependence, No hurts**. (Epoll is native support for linux.)

## Example

### configure & compile

```
mkdir build;
cd build
cmake ..
make
```

### start server

```
make && ./husky.demo
```

### GET request

```
curl "http://127.0.0.1:11257/?hello=world&myname=aszxqw"
```

### POST request

```
curl -d "hello world, my name is aszxqw." "http://127.0.0.1:11257"
```

## Contact

wuyanyi09@foxmail.com

