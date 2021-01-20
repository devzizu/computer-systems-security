#!flask/bin/python
import sys

from flask import Flask
from flask_cors import CORS, cross_origin
from flask import jsonify
from flask import request

app = Flask(__name__)
CORS(app, support_credentials=True)

#------------------------------------------

PENDING_CODES = {}
PENDING_CODES[str(0)] = "default"

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
def all_codes():

    if (request.args.get('id') is None):
        return jsonify({ "result": { "pending": PENDING_CODES } })
    
    codeN = request.args.get('id')
    
    if (codeN in PENDING_CODES):
        return jsonify({ "result" : PENDING_CODES[codeN] })

    return jsonify({ "result" : "Code {} not found!".format(codeN) })
    
#------------------------------------------

@app.route(ROUTE_RESOURCE_CODE, methods=['POST'])
@cross_origin(supports_credentials=True)
def validate_code():
    
    codeN = request.json.get('code')
    codeD = request.json.get('decision')

    codeNStr = str(codeN)
    
    if (codeNStr in PENDING_CODES):
        return jsonify({ "result": { "code": codeN, "status": "already registred" } }), 400
    
    if (not (codeD == "authorize") and not (codeD == "deny")):
        return jsonify({ "result": { "code": codeNStr, "status": "please specify a valid decision (ex: 'deny' or 'authorize')" } }), 400

    PENDING_CODES[codeNStr] = codeD

    return jsonify({ "result": { "code": codeNStr, "status": "registred" } }), 201

#------------------------------------------

if __name__ == '__main__':
    app.run(debug = True)
