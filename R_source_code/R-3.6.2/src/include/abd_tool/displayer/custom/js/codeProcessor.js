var selectedLine;

var setSelectedCSS = 'container-fluid overflow-auto clickable-content selected-content';
var setUnselectedCSS = 'container-fluid overflow-auto clickable-content content';

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

function setSelected(line) {
	var clickedElement = document.getElementById(line);
	clearWantDisplay();
	//when user interacts with some line

	if (selectedLine == line) {
		//selected the same
		selectedLine = '';
		clickedElement.className = setUnselectedCSS;
		//clear panes
		clearPanes();
	} else if (selectedLine == '') {
		// selected without anything selected
		//no need to clear, nothing selected
		selectedLine = line;
		clickedElement.className = setSelectedCSS;
	} else {
		//changed the selected line (with one already selected)
		document.getElementById(selectedLine).className = setUnselectedCSS;
		selectedLine = line;
		clickedElement.className = setSelectedCSS;
	}

	//now invoke the events processor
	if (selectedLine != '') processForLine(line);
}

function clearPanes() {
	populateUpperLabels(-1, startingEnv);
	clearEventPane();
	clearObjectPane();
}
