function acceptCode(state) {

	document.getElementById('inputCode').value = "";

	fetch("http://localhost:5000/auth-fs/api/codes", {
	  method: "post",
	  headers: {
	    'Accept': 'application/json',
	    'Content-Type': 'application/json;charset=UTF-8'
	  },

	  body: JSON.stringify({
	    "code": document.getElementById('inputCode').value,
	    "authorization": state
	  })
	})
	.then( (response) => { 

	   showModel(true);
	   
	})
	.catch(function() {

		showModel(false);

	});

}

function showModel(state){

	var modal = document.getElementById("myModal");

	var modal_header = document.getElementById("modal-header");

	var estadoPop = document.getElementById("estadoPop");
	
	if(!state){

		//False

		modal_header.style.backgroundColor = "red"
		
		estadoPop.textContent = "Error"

		textoPop.textContent = "There was a problem completing your request..."

		console.log(state);
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

	var maxLength = 10;
    
    if(value.length >= maxLength) return false;
    
    return true;
}

document.getElementById('inputCode').onkeyup = function(){

    if(!alertMaxSizeCode(this.value)) this.style.color = "red";
    else this.style.color = "black";

}
