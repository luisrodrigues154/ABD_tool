var selectedLine;

var setSelectedCSS = 'container-fluid overflow-auto clickable-content selected-content';
var setUnselectedCSS = 'container-fluid overflow-auto clickable-content content';

$(function() {
	//loading function
	selectedLine = '';
	loadCode();
});

//<label class="container-fluid overflow-auto clickable-content content" onClick="setSelected(this.id)" id="line-1">1:&nbsp;&nbsp;&nbsp;a <- 30</label> </label>
function loadCode() {
	var htmlProduced = '';
	// var codeArray = code['code'];
	// alert('code len: ' + codeArray.length);
	var line = 1;
	code.forEach((element) => {
		//add the head
		htmlProduced +=
			'<label class="container-fluid overflow-auto clickable-content content" onClick="setSelected(this.id)"';
		//add the id to the label
		htmlProduced += 'id="line-' + line + '">';
		//add line number and the space between line number and code itself
		htmlProduced += line + ':&nbsp;&nbsp;&nbsp;';
		//add the code
		htmlProduced += element;
		//add the tail
		htmlProduced += '</label>';
		line++;
	});

	document.getElementById('code_container').innerHTML = htmlProduced;
}

function setSelected(id) {
	var clickedElement = document.getElementById(id);

	//when user selects a new line
	if (selectedLine == id) {
		selectedLine = '';
		clickedElement.className = setUnselectedCSS;
		//clear panes
	} else if (selectedLine == '') {
		//no need to clear, nothing selected
		selectedLine = id;
		clickedElement.className = setSelectedCSS;
	} else {
		document.getElementById(selectedLine).className = setUnselectedCSS;
		selectedLine = id;
		clickedElement.className = setSelectedCSS;
	}

	//now invoke the events processor
}
