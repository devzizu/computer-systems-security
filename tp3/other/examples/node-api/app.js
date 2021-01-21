var express = require("express");
var app = express();
var bodyParser = require("body-parser");

var codesMap = {}
codesMap[0] = "deny"

app.use(bodyParser.json())

app.listen(3000, () => {
    console.log("Server running on port 3000");
});

app.post("/confirm", function (request, response) {
    
    body = request.body
    
    if (body.code in codesMap) {
        response.status(400).send(JSON.stringify({"result": "Code already confirmed!"}))
    } else {
        codesMap[body.code] = body.decision
        response.status(200).send(JSON.stringify({"result": "Confirmation accepted!"}))
    }

    console.log(codesMap)
});

app.get("/code", function(request, response) {
    
    response.setHeader('Content-Type', 'application/json');

    code = request.query.id
    if (code in codesMap) {
        response.status(200).send(JSON.stringify({"result": codesMap[code]}))
    } else {
        response.status(404).send(JSON.stringify({"result": "Not Found"}))
    }
});
