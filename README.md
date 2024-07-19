[![Test](https://github.com/yanyiwu/husky/actions/workflows/test.yml/badge.svg)](https://github.com/yanyiwu/husky/actions/workflows/test.yml)
- - -

# husky

## Introduction

husky is a **Simple** HTTP Server Frame Based on ThreadPool.

It is just a simple server, not forcussing on performance, but it is **very easy** to use. That is what it born to be.

## Feature

+ Only Headers file: what you need to do is `include` it.
+ No dependence: **No dependence, No hurts**.

## Example

### configure & compile

```
git clone --recurse-submodules https://github.com/yanyiwu/husky.git
cd husky
mkdir build
cd build
cmake ..
make
```

### start server

```
./threadpoolserver --port 11257
```

### GET Request Example

```
curl "http://127.0.0.1:11257/?hello=world&myname=yanyiwu"
```

### POST Request Example
```
curl -d "hello world, my name is yanyiwu." "http://127.0.0.1:11257"
```


### husky.demo

Its source code is `test/demo.cpp`.  Its code has only 30+ lines. 

## Benchmark

```
go get github.com/yanyiwu/go_http_load
```

GET

```
go_http_load -method=GET -get_urls="../test/testdata/get.urls" -goroutines=1 -loop_count=5000
```

```
The Number of Queries:10000
The Time Consumed: 4.539 s
Query Per Second: 2203.046 q/s
```

POST

```
go_http_load -method=POST -post_url="http://127.0.0.1:11257" -post_data_file="../test/testdata/post.data" -goroutines=1 -loop_count=5000
```

```
The Number of Queries:5000
The Time Consumed: 2.448 s
Query Per Second: 2042.887 q/s
```

## Reference

[limonp]

[limonp]:https://github.com/yanyiwu/limonp.git
