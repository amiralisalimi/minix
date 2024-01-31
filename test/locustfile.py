from locust import HttpUser, between, task


class WebsiteUser(HttpUser):
    wait_time = between(5, 15)

    @task
    def test_port_one(self):
        self.client.get("/nine/")

    @task
    def test_port_two(self):
        self.client.get("/nine/nine/")

    @task
    def test_static_one(self):
        self.client.get("/static/")

    @task
    def test_static_two(self):
        self.client.get("/static/sec/")
