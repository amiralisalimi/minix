# Mini NGINX

## Quick Setup:

### Build:
```shell
    make build
```

### Build and Run:
By running this command the Mini-Nginx will listen to port 8000:
```shell
    make run
```
The `config.txt` will be created automatically, if it does not exist.
You can declare your routing rules in the `config.txt` file.
A basic example of `config.txt` is:
```text
/nine/          ->      9000
/nine/nine/     ->      9090
/static/        ->      /home/ali/sta1/
/static/sec/    ->      /home/ali/sta2/
```

<br/>

## Locust Load test:

Install locust if not installed before by:
```shell
    pip install locust
```
Then run this command:
```shell
    make load-test
```
Then, set the host to `http://0.0.0.0:8000`, in the locust webpage.

<br/>

## Some test apps:

### Run a Django App on port 9000 by:
First create a Django project by:
```shell
    make django-init
```
Then run this command to start django on port 9000:
```shell
    make django-run
```


### Run a python Http-server on port 9090 by:
```shell
    make http-server-run
```

