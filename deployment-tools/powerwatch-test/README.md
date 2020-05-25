PowerWatch Test
=====

Made to test running powerwatches with a bardcode scanner. Only
checks devices in a specific product

Prints the powerwatch firmware version, whether it is claimed or not
and a bunch of metrics from the database about the powerwatch device being tested

Requires test-config.json to point to the right database and such

## Usage

### Install dependencies

```
$ pip3 install psycopg2
$ pip3 install requests
$ pip3 install pyyaml
```

### Get the configuration file

The test script relies on a config file that contains secrets. 
The easiest way to get the file is from cloud secrets, which requires
installing the gcloud CLI (https://cloud.google.com/sdk/install). After install:

```
$ gcloud secrets versions access latest --secret powerwatch-test-ghana > test-config.json
```

### Run the script

Run

```
$ ./test.py
```

And scan a QR code with the barcode scanner. If all the fields are green then
the powerwatch has passed the test! The powerwatch probably needs to be running for a while before it will pass.
