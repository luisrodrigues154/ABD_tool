var startingEnv;
var baseFunction;

var currContext = '';
var currFunc = '';
var requestDisplay = [];

const types = {
	ASSIGN: 'assign_event',
	IF: 'if_event',
	FUNC: 'func_event',
	RET: 'ret_event',
	ARITH: 'arith_event',
	VEC: 'vector_event',
	IDX: 'idx_change_event',
	FOR: 'for_loop_event',
	BREAK: 'break_event',
	NEXT: 'next_event',
	REPEAT: 'repeat_loop_event'
};
var processMe = 0;

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
	if (currContext != predecessorContext && requestLine != '') {
		toggleLineSelect('line-' + requestLine, 1);
		predecessorContext = currContext;
	}

	if (currContext === undefined) {
		findFirstEvent();
		predecessorContext = currContext;
		if (requestLine != '') {
			toggleLineSelect('line-' + requestLine, 0);
		}
	}
}

function applyContext() {
	var htmlProduced = '';
	if (modalShown) {
		if (modalSelectedLine == '') {
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
			htmlProduced += processEventByType(event);
		}
	});

	verifyTrack(requestDisplay);
	document.getElementById('events_container').innerHTML = htmlProduced + fakeBottomHr;
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
	requestLine = trimLine;
	for (i = 1; i <= numEvents; i++) {
		var currentLine = events[i]['line'];
		if (currentLine == requestLine) {
			foundEvents.push(events[i]);
			if (!foundContexts.includes(events[i]['atEnv'])) {
				foundContexts.push(events[i]['atEnv']);
			}
		}
	}
	console.log('foundContexts: ' + foundContexts.length);
	if (foundContexts.length > 1) {
		//prompt to choose
		promptForContext(foundEvents[0]['atFunc'], foundContexts);
	} else if (foundContexts.length > 0) {
		currContext = foundContexts[0];

		checkContextChange();
		applyContext();
	} else {
		toggleLineSelect(line, 0);
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

function checkMoreVectorsForSameLine(line, context) {
	var i;
	var count = 0;
	for (i = 1; i <= Object.keys(events).length; i++) {
		//fast check the line
		if (events[i]['line'] == line) {
			//check if first if vector (cost less than compare the context first)
			if (events[i]['type'] == types.VEC) {
				//finally check the context
				if (events[i]['atEnv'] == context) {
					count++;
					if (count == 2) return true;
				}
			}
		}
	}
	return false;
}

function processEventByType(event) {
	var htmlProduced = '';

	switch (event['type']) {
		case types.ASSIGN:
			htmlProduced += processAssignEvent(event) + hr;
			break;
		case types.IF:
			break;
		case types.FUNC:
			htmlProduced += processFunCallEvent(event) + hr;
			break;
		case types.RET:
			htmlProduced += processReturnEvent(event);
			break;
		case types.ARITH:
			break;
		case types.VEC:
			if (
				(event['data']['toObj'] == false && !checkMoreVectorsForSameLine(event['line'], event['atEnv'])) ||
				(event['data']['toObj'] == true && processMe)
			) {
				htmlProduced += processVecEvent(event);
				if (event['data']['toObj'] == false) {
					htmlProduced += hr;
				}
			}
			processMe = 0;
			break;
		default:
			break;
	}

	return htmlProduced;
}

var HC_text = 'User-Typed';
var R_text = 'Non-Tracked Object';

function genEventCol(colName, colContent) {
	var htmlProduced = '';

	htmlProduced += '<div class="col col-md-auto col1">';
	htmlProduced += '<label class="event-content">' + colName + '</label>';
	colContent.forEach((element) => {
		htmlProduced += '<label class="event-rhs d-block ">' + element + '</label>';
	});

	htmlProduced += '</div>';

	return htmlProduced;
}

function generateEventRow(eventTitle, eventContent) {
	var htmlProduced = '';
	htmlProduced += '<label class="d-block event-title" id="event_title">' + eventTitle + '</label>';
	htmlProduced += '<div class="container-fluid" id="event_content">';
	htmlProduced += '<div class="row">';
	htmlProduced += eventContent;
	htmlProduced += '</div>';
	htmlProduced += '</div>';
	return htmlProduced;
}

function getEventById(eventId) {
	return events[eventId];
}

function processVecEvent(event) {
	var eventContent = '';
	var colContent = [];
	var rangeL = event['data']['rangeL'];
	var rangeR = event['data']['rangeR'];
	var toObj = event['data']['toObj'];
	var fromObj = event['data']['fromObj'];

	let toObjToDisplay = {
		id: '',
		name: '',
		withIndex: '',
		state: ''
	};

	if (!toObj) {
		colContent.push('Creation');
		eventContent += genEventCol('Action', colContent);
		colContent = [];
		colContent.push(String(event['data']['size']));
		eventContent += genEventCol('Size', colContent);
		colContent = [];
	} else {
		colContent.push('Vector Creation');
		eventContent += genEventCol('From event', colContent);
		colContent = [];
		colContent.push(String(event['data']['size']));
		eventContent += genEventCol('Size', colContent);
		colContent = [];
	}

	if (fromObj != 0) {
		//from obj
		var objName = getCommonObjNameById(fromObj);
		colContent.push(objName);
		eventContent += genEventCol('Created from', colContent);
		colContent = [];

		toObjToDisplay.id = fromObj;
		toObjToDisplay.name = objName;
		toObjToDisplay.state = event['data']['fromState'];
		toObjToDisplay.withIndex = -1;
		requestObjDisplay(toObjToDisplay);
	}

	if (!(rangeL == -1001 && rangeR == -1001)) {
		colContent.push(String(rangeL) + ':' + String(rangeR));
		eventContent += genEventCol('With range', colContent);
		colContent = [];
	}

	if (!toObj) {
		return generateEventRow('Vector creation', eventContent);
	} else {
		return eventContent;
	}
}
function processFunCallEvent(event) {
	var eventContent = '';
	var funName = getCodeFlowObjNameById(event['data']['toId']);
	var colContent = [];
	var rcvdArgs = [];
	//funtion
	colContent.push(String(funName) + '()');
	eventContent += genEventCol('Name', colContent);
	colContent = [];

	//arguments
	var args = event['data']['args'];

	args.forEach((arg) => {
		let toObjToDisplay = {
			id: '',
			name: '',
			withIndex: '',
			state: ''
		};
		let fromObjToDisplay = {
			id: '',
			name: '',
			withIndex: '',
			state: ''
		};
		var fromId = arg['fromId'];
		if (fromId == -1) {
			colContent.push(HC_text);
		} else if (fromId == -2) {
			colContent.push(R_text);
		} else {
			var objName = getCommonObjNameById(fromId);
			fromObjToDisplay.id = arg['fromId'];
			fromObjToDisplay.name = objName;
			fromObjToDisplay.withIndex = -1;
			fromObjToDisplay.state = arg['fromState'];
			colContent.push(objName);
			requestObjDisplay(fromObjToDisplay);
		}
		toObjToDisplay.id = arg['toId'];
		toObjToDisplay.name = getCommonObjNameById(toObjToDisplay.id);
		toObjToDisplay.withIndex = -1;
		toObjToDisplay.state = arg['toState'];
		rcvdArgs.push(toObjToDisplay.name);
		requestObjDisplay(toObjToDisplay);
	});
	eventContent += genEventCol('Args passed', colContent);
	colContent = [];
	eventContent += genEventCol('Args Received', rcvdArgs);

	return generateEventRow('Function Call', eventContent);
}

function processReturnEvent(event) {
	var funcName = getCodeFlowObjNameById(event['data']['fromId']);
	var colContent = [];
	//return is integrated with assignment, so just need to generate its column
	colContent.push(String(funcName) + ' () return');

	return genEventCol('From', colContent);
}

function processAssignEvent(event) {
	var eventContent = '';
	var colContent = [];

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

	//to
	colContent.push(toObjToDisplay.name);
	eventContent += genEventCol('To object', colContent);
	colContent = [];

	//from

	if (event['data']['origin'] == 'obj') {
		if (event['data']['fromObj'] == 'HC') {
			colContent.push(HC_text);
		} else if (event['data']['fromObj'] == 'R') {
			colContent.push(R_text);
		} else {
			fromObjToDisplay.id = event['data']['fromId'];
			fromObjToDisplay.state = event['data']['fromState'];
			fromObjToDisplay.name = getCommonObjNameById(fromObjToDisplay.id);
			fromObjToDisplay.withIndex = event['data']['withIndex'];
			colContent.push(fromObjToDisplay.name);
		}
		eventContent += genEventCol('From object', colContent);
	} else {
		var event = getEventById(event['data']['fromEvent']);
		processMe = 1;
		eventContent += processEventByType(event);
	}

	if (fromObjToDisplay.id != 0) {
		requestObjDisplay(fromObjToDisplay);
	}
	requestObjDisplay(toObjToDisplay);

	return generateEventRow('Assignment', eventContent);
}

function requestObjDisplay(id) {
	if (!requestDisplay.includes(id)) {
		requestDisplay.push(id);
	}
}
