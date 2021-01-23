#!flask/bin/python

import re
import sys
import time
import threading
import atexit
import datetime

from apscheduler.schedulers.background import BackgroundScheduler

from flask import Flask
from flask_cors import CORS, cross_origin
from flask import jsonify
from flask import request

from flask_limiter import Limiter
from flask_limiter.util import get_remote_address

TIME_FLUSH = 1
DELTA_SUBT = 40

app = Flask(__name__)
CORS(app, support_credentials=True)

limiter = Limiter(
    app,
    key_func=get_remote_address,
    default_limits=["6 per minute"]
)

#------------------------------------------

PENDING_CODES = {}
PENDING_CODES[str(0)] = {"decision": "<default>", "timestamp": datetime.datetime.now()}

ROUTE_BASE      = "/auth-fs/api"
ROUTE_RESOURCE_CODE  = "{}/codes".format(ROUTE_BASE)

#------------------------------------------

@app.route('/')
def index():
    return jsonify({ 
        "routes": { 
            "GET/": ROUTE_RESOURCE_CODE, 
            "POST /" : ROUTE_RESOURCE_CODE 
        } 
    })

#------------------------------------------

@app.route(ROUTE_RESOURCE_CODE, methods=['GET'])
@limiter.limit("60/minute")
def all_codes():

    if (request.args.get('id') is None):
        return jsonify({ "result": { "pending": PENDING_CODES } })
    
    codeN = request.args.get('id')
    
    if (codeN in PENDING_CODES):
        return jsonify({ "result" : PENDING_CODES[codeN].get("decision") })

    return jsonify({ "result" : "Code {} not found!".format(codeN) })
    
#------------------------------------------

@app.route(ROUTE_RESOURCE_CODE, methods=['POST'])
@cross_origin(supports_credentials=True)
@limiter.limit("6/minute")
def validate_code():
    
    codeN = request.json.get('code')
    codeD = request.json.get('decision')

    codeNStr = str(codeN)

    matched = bool(re.match("[a-zA-Z0-9]{10}", codeNStr)) 
    print(matched and len(codeNStr) != 10)
    if ((len(codeNStr) != 10) or not matched):
        return jsonify({ "result": { "code": codeN, "status": "bad code syntax" } }), 400
        
    if (codeNStr in PENDING_CODES):
        return jsonify({ "result": { "code": codeN, "status": "already registred" } }), 400
    
    if (not (codeD == "authorize") and not (codeD == "deny")):
        return jsonify({ "result": { "code": codeNStr, "status": "please specify a valid decision (ex: 'deny' or 'authorize')" } }), 400

    PENDING_CODES[codeNStr] = { "decision": codeD, "timestamp": datetime.datetime.now() }

    return jsonify({ "result": { "code": codeNStr, "status": "registred" } }), 201

#------------------------------------------

def flush_cache():
    print("[flushing] +",TIME_FLUSH,"seconds passed...[",time.strftime("%A, %d. %B %Y %I:%M:%S %p"),"]")
    timeS = datetime.datetime.now()
    deleteElements = []
    for code in PENDING_CODES:
        tstamp = PENDING_CODES[code].get("timestamp")
        if (tstamp < (timeS - datetime.timedelta(seconds=DELTA_SUBT))):
            deleteElements.append(code)

    for elem in deleteElements:
        del PENDING_CODES[elem]

if __name__ == '__main__':
    
    scheduler = BackgroundScheduler()
    scheduler.add_job(func=flush_cache, trigger="interval", seconds=TIME_FLUSH)
    scheduler.start()
    atexit.register(lambda: scheduler.shutdown())

    app.run(debug = False)
