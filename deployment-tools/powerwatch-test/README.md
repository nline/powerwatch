PowerWatch Test
=====

Made to test running powerwatches with a bardcode scanner. 

## Usage

### Install dependencies

#### Google Cloud
You must have permissions to access database secrets to run this code.

Install gcloud: https://cloud.google.com/sdk/docs/install

```
$ gcloud auth login
```

#### Python dependencies
```
$ pip3 install pipenv
$ pipenv install
```

### Run the script

Run

```
$ pipenv run ./test.py
```

And scan a QR code with the barcode scanner. If all the fields are green then
the powerwatch has passed the test! The powerwatch needs to be running for 2 hours before it will pass.
