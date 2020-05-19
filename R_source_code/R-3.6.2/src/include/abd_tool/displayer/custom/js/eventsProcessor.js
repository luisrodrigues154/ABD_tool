var startingEnv;
var baseFunction;

var currContext = '';
var currFunc = '';
var requestDisplay = [];

$(function() {
	//loading function
	$('#context_modal').on('hidden.bs.modal', function() {
		applyContext();
	});
});
function clearEventPane() {
	document.getElementById('events_container').innerHTML = '';
}

function getUpperLabelInit() {
	var data = [];
	data.push(baseFunction);
	data.push(startingEnv);
	return data;
}

function findFirstEvent() {
	var foundEvents = [];
	if (Object.keys(events).length > 0) {
		foundEvents.push(events[1]);
		baseFunction = 'main';
		startingEnv = events[1]['atEnv'];
	} else {
		startingEnv = 'NA';
		baseFunction = 'NA';
	}

	currContext = startingEnv;
	predecessorContext = currContext;
	return foundEvents;
}

function radioSelected(radioLine, radioContext) {
	modalSelectedLine = radioLine;
	modalSelectedContext = radioContext;
}
var predecessorLine = '';
var requestLine = '';
var modalShown = 0;
var foundEvents = [];
var modalSelectedLine = '';
var modalSelectedContext = '';
var predecessorContext = '';

function clearModalInfo() {
	if (predecessorLine != '') {
		togglePredecessorSelect(predecessorLine, 0);
	}
	if (modalSelectedLine != '') {
		toggleLineSelect(modalSelectedLine, 0);
	}
	predecessorLine = '';
	modalSelectedLine = '';
	requestLine = '';
	modalShown = 0;
	foundEvents = [];
}
function checkContextChange() {
	if (currContext != predecessorContext && checkedLine != '') {
		document.getElementById('line-' + checkedLine).className = setUnselectedCSS;
		predecessorContext = currContext;
	}

	if (currContext === undefined) {
		findFirstEvent();

		predecessorContext = currContext;
		if (requestingLine != '') {
			document.getElementById('line-' + requestingLine).className = setUnselectedCSS;
		}
	}
}

function applyContext() {
	var htmlProduced = '';
	if (modalShown) {
		if (modalSelectedLine == '') {
			console.log('here');
			toggleLineSelect(requestLine, 0);
			clearModalInfo();
			findFirstEvent();
			return;
		} else {
			togglePredecessorSelect(modalSelectedLine, 1);
			currContext = modalSelectedContext;
		}
	}

	requestDisplay = [];
	if (foundEvents.length > 0) {
		populateUpperLabels(foundEvents[0]['atFunc'], currContext);
	}
	foundEvents.forEach((event) => {
		if (event['atEnv'] == currContext) {
			htmlProduced = processEventByType(event);
		}
	});
	verifyTrack(requestDisplay);
	document.getElementById('events_container').innerHTML = htmlProduced;
}

function produceRadioButton(line, id, context) {
	var htmlProduced = '';
	htmlProduced += '<div class="form-check">';
	htmlProduced += '<label class="form-check-label" for="ctxtRadio-' + id + '" >';
	htmlProduced += '<input class="form-check-input" type="radio" name="contextRadio"';
	htmlProduced += ' id="ctxtRadio-' + id + '" value="option' + id;
	htmlProduced += '" onclick="radioSelected(\'line-' + line + "', '" + String(context) + '\')">';
	htmlProduced += 'line ' + document.getElementById('line-' + line).textContent.replace(/\s/g, '');
	htmlProduced += '</label>';
	htmlProduced += '</div>';
	return htmlProduced;
}

function findFuncEventsForId(funcId, currEnv) {
	//returns the line numbers
	var numEvents = Object.keys(events).length;
	var i;
	var count = 0;
	var htmlProduced = '';
	for (i = 1; i <= numEvents; i++) {
		if (events[i]['type'] == types.FUNC && events[i]['atEnv'] == currEnv) {
			if (events[i]['data']['toId'] == funcId) {
				count++;
				htmlProduced += produceRadioButton(String(events[i]['line']), count, events[i]['data']['toEnv']);
			}
		}
	}
	return htmlProduced;
}

function promptForContext(funcId, contexts) {
	var htmlProduced = findFuncEventsForId(funcId, currContext);
	document.getElementById('context_modal_body').innerHTML = htmlProduced;
	modalShown = 1;
	$('#context_modal').modal('show');
}

function processForLine(line) {
	var trimLine = line.split('-')[1];
	var numEvents = Object.keys(events).length;
	var foundContexts = [];

	var i;
	clearModalInfo();
	requestLine = line;
	for (i = 1; i <= numEvents; i++) {
		var currentLine = events[i]['line'];
		if (currentLine == trimLine) {
			foundEvents.push(events[i]);
			if (!foundContexts.includes(events[i]['atEnv'])) {
				foundContexts.push(events[i]['atEnv']);
			}
		}
	}

	if (foundContexts.length > 1) {
		//prompt to choose
		promptForContext(foundEvents[0]['atFunc'], foundContexts);
	} else {
		currContext = foundContexts[0];
		checkContextChange();
		applyContext();
	}
}

function populateUpperLabels(funcId, env) {
	if (funcId == -1) {
		document.getElementById('func_name_label').innerHTML = baseFunction;
	} else {
		document.getElementById('func_name_label').innerHTML = getCodeFlowObjNameById(funcId);
	}

	document.getElementById('env_label').innerHTML = env;
}

const types = {
	ASSIGN: 'assign_event',
	IF: 'if_event',
	FUNC: 'func_event',
	RET: 'ret_event',
	ARITH: 'arithmetic_event',
	VEC: 'vector_event'
};

function processEventByType(event) {
	var htmlProduced = '';

	switch (event['type']) {
		case types.ASSIGN:
			htmlProduced = produceASSIGNhtml(event);
			break;
		case types.IF:
			break;
		case types.FUNC:
			break;
		case types.RET:
			break;
		case types.ARITH:
			break;
		case types.VEC:
			break;

		default:
			break;
	}

	return htmlProduced;
}

var labelHeadTitle = '<label class="d-block event-title">';

var labelRhs = '<label class="event-rhs">';
var labelHeadContent = '<label class="d-block event-content ml-2">';
var labelTail = '</label>';

var HC_text = 'User-Typed';
var R_text = 'Non-Tracked Object';

function produceASSIGNhtml(event) {
	var htmlProduced = '';

	let toObjToDisplay = {
		id: '',
		name: '',
		withIndex: '',
		state: ''
	};

	let fromObjToDisplay = {
		id: '0',
		name: '',
		withIndex: '',
		state: ''
	};

	toObjToDisplay.id = event['data']['toObj'];
	toObjToDisplay.name = getCommonObjNameById(String(toObjToDisplay.id));
	toObjToDisplay.state = event['data']['toState'];
	toObjToDisplay.withIndex = -1;

	//title
	htmlProduced += labelHeadTitle;
	htmlProduced += 'Assignment';
	htmlProduced += labelTail;

	//to
	htmlProduced += labelHeadContent;
	htmlProduced += 'To Object: ';
	htmlProduced += labelRhs;

	htmlProduced += toObjToDisplay.name;
	htmlProduced += labelTail;
	htmlProduced += labelTail;

	//from
	htmlProduced += labelHeadContent;

	if (event['data']['origin'] == 'obj') {
		htmlProduced += 'From Object: ';
		htmlProduced += labelRhs;
		if (event['data']['fromObj'] == 'HC') {
			htmlProduced += HC_text;
		} else if (event['data']['fromObj'] == 'R') {
			htmlProduced += R_text;
		} else {
			fromObjToDisplay.id = event['data']['fromId'];
			fromObjToDisplay.state = event['data']['fromState'];
			fromObjToDisplay.name = getCommonObjNameById(fromObjToDisplay.id);
			fromObjToDisplay.withIndex = event['data']['withIndex'];
			htmlProduced += fromObjToDisplay.name;
		}
	} else {
		htmlProduced += 'From Event: ';
		htmlProduced += labelRhs;
	}

	if (fromObjToDisplay.id != 0) {
		requestDisplay.push(fromObjToDisplay);
	}
	requestDisplay.push(toObjToDisplay);

	htmlProduced += labelTail;
	htmlProduced += labelTail;
	return htmlProduced;
}
