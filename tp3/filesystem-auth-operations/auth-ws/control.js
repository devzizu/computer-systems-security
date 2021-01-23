
var maxLength = 10;

async function acceptCode(state) {

    inputCodeString = document.getElementById('inputCode').value;

    if (inputCodeString.length != maxLength) {
        showModal("inputerr"); 
        return;
    } 
    
    if ((/^([a-zA-Z0-9]{10})$/.test(inputCodeString)) == false) {
        showModal("invalidinputerr"); 
        return;    
    }

    dec = state == true ? "authorize" : "deny";
    json = JSON.stringify({
	    "code": document.getElementById('inputCode').value,
	    "decision": dec
	  })

    show = "default"

    await fetch("http://localhost:5000/auth-fs/api/codes", {
	  method: "post",
	  headers: {
	    'Accept': 'application/json',
	    'Content-Type': 'application/json;charset=UTF-8'
	  },
	  body: json 	
    })
	.then( (response) => { 
    
	    if (response.status > 201)
            show = "errorres";
        else 
            show = "ok";
	}).catch((error) => {
        show = "errorcon";
    });

    showModal(show)

	document.getElementById('inputCode').value = "";
}

function showModal (state){

	var modal = document.getElementById("myModal");
	var modal_header = document.getElementById("modal-header");
	var estadoPop = document.getElementById("estadoPop");

	if(state === "errorres"){

		modal_header.style.backgroundColor = "red"
		estadoPop.textContent = "Error"
		textoPop.textContent = "There was a problem completing your request..."

    } else if (state === "ok"){

        modal_header.style.backgroundColor = "green"
		estadoPop.textContent = "Success!"
		textoPop.textContent = "Your request was submitted..."
    
    } else if (state === "errorcon") {
            modal_header.style.backgroundColor = "DarkKhaki"
		            estadoPop.textContent = "Oooops!"
                                textoPop.textContent = "API is currently down, try again later..."
    } else                   if (state==="inputerr") {
         modal_header.style.backgroundColor = "DarkKhaki"
		            estadoPop.textContent = "Oooops, invalid code!"
                                textoPop.textContent = "All codes should have " + maxLength+ " chars..."   
    } else if (state === "invalidinputerr") {
          modal_header.style.backgroundColor = "DarkKhaki"
		  estadoPop.textContent = "Oooops, invalid code!"
          textoPop.textContent = "Valid codes have chars and numbers only..."   
    }

	var span = document.getElementsByClassName("close")[0];

	modal.style.display = "block";

	span.onclick = function() {
	  modal.style.display = "none";
	}

	window.onclick = function(event) {
	  if (event.target == modal) {
	    modal.style.display = "none";
	  }
	}
}

function alertMaxSizeCode(value){ 
    if(value.length != maxLength) return false;
    
    return true;
}

document.getElementById('inputCode').onkeyup = function(){

    if(!alertMaxSizeCode(this.value)) this.style.color = "red";
    else this.style.color = "green";
}
