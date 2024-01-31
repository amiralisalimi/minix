# Mini NGINX

## Quick Setup:

### Build
```shell
    make build
```

### Run
By running this command the Mini-Nginx will listen to port 8000:
```shell
    make run
```

<br/>

## Locust Load test:
First run this command:
```shell
    make load-test
```
Then, set the host to `http://0.0.0.0:8000`, in the locust webpage.

<br/>

## Some test apps:

### Run a Django App on port 9000 by:
```shell
    make django-run
```

Note: To run Django you should run `make django-init` once.


### Run a python Http-server on port 9090 by:
```shell
    make http-server-run
```

