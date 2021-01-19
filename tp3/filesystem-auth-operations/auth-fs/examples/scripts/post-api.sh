
echo $1 " " $2
curl --request POST \
    --url 'http://localhost:3000/confirm' \
    --header 'content-type: application/json' \
    --data '{"code": '"$1"', "decision": '"\"$2\""'}'

