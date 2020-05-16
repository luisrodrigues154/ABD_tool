var eventsContainer;

$(function() {
	//loading function
});
function clearEventPane() {
	document.getElementById('events_container').innerHTML = '';
}

function findEvents(line) {
	var numEvents = Object.keys(events).length;
	var foundEvents = [];
	var i;

	for (i = 1; i <= numEvents; i++) {
		var currentLine = events[i]['line'];
		if (currentLine > line) break;

		if (currentLine == line) foundEvents.push(events[i]);
	}

	return foundEvents;
}
var requestDisplay = [];

function processForLine(line) {
	var foundEvents = findEvents(line.split('-')[1]);
	var htmlProduced = '';
	requestDisplay = [];
	foundEvents.forEach((event) => {
		htmlProduced = processEventByType(event);
	});
	verifyTrack(requestDisplay);
	document.getElementById('events_container').innerHTML = htmlProduced;
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
	toObjToDisplay.name = getObjNameById(toObjToDisplay.id);
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
		} else if (event['data']['fromObj'] == 'HC') {
			htmlProduced += R_text;
		} else {
			fromObjToDisplay.id = event['data']['fromId'];
			fromObjToDisplay.state = event['data']['fromState'];
			fromObjToDisplay.name = getObjNameById(fromObjToDisplay.id);
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
