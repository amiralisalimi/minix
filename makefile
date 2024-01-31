build:
	gcc src/config.c src/main.c src/static.c -g -o main
run:
	gcc src/config.c src/main.c src/static.c -g -o main && ./main

clean:
	rm main

django-init:
	python -m venv test/basic-django-app/.venv && \
 	. test/basic-django-app/.venv/bin/activate && \
	pip install -r test/basic-django-app/requirements.txt && \
	test/basic-django-app/.venv/bin/python test/basic-django-app/manage.py migrate && \
	deactivate

django-run:
	test/basic-django-app/.venv/bin/python test/basic-django-app/manage.py runserver 9000

http-server-run:
	python -m http.server 9090

load-test:
	(echo Note that you sholud run django first)
	locust -f test/locustfile.py
