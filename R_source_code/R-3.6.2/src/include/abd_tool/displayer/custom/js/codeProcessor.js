var selectedLine;

var setSelectedCSS = 'container-fluid overflow-auto clickable-content selected-content';
var setUnselectedCSS = 'container-fluid overflow-auto clickable-content content';
var setSelectedPredecessor = 'container-fluid overflow-auto clickable-content content selected-predecessor';
$(function() {
	//loading function
	selectedLine = '';
	loadCode();
});

function loadCode() {
	var htmlProduced = '';
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
	var foundEvents = findFirstEvent();
	if (foundEvents.length > 0) {
		populateUpperLabels(foundEvents[0]['atFunc'], foundEvents[0]['atEnv']);
	} else {
		populateUpperLabels(-1, startingEnv);
	}

	document.getElementById('code_container').innerHTML = htmlProduced;
}
function clearSelectedLine() {
	selectedLine = '';
}

function togglePredecessorSelect(line, select) {
	if (select) {
		document.getElementById(line).className = setSelectedPredecessor;
	} else {
		document.getElementById(line).className = setUnselectedCSS;
	}
}

function toggleLineSelect(line, select) {
	if (select) {
		document.getElementById(line).className = setSelectedCSS;
	} else {
		//selectedLine = '';
		document.getElementById(line).className = setUnselectedCSS;
	}
}

function setSelected(line) {
	clearWantDisplay();
	//when user interacts with some line
	console.log('inside: ' + line);
	console.log('selected: ' + selectedLine);
	if (selectedLine == line) {
		//selected the same
		selectedLine = '';
		toggleLineSelect(line, 0);
		//clear panes
		clearPanes();
		clearModalInfo();
		findFirstEvent();
	} else if (selectedLine == '') {
		// selected without anything selected
		//no need to clear, nothing selected
		selectedLine = line;
		toggleLineSelect(line, 1);
	} else {
		//changed the selected line (with one already selected)
		toggleLineSelect(selectedLine, 0);
		console.log('got here');
		selectedLine = line;
		toggleLineSelect(selectedLine, 1);
		console.log('Selected line: ' + selectedLine);
	}

	//now invoke the events processor
	if (selectedLine != '') processForLine(line);
}

function clearPanes() {
	populateUpperLabels(-1, startingEnv);
	clearEventPane();
	clearObjectPane();
}
