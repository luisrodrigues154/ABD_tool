var stack = [];
var stackSize = 0;
var currentEnv = '';
var valuesToBigData = null;
let eventsLen = 0;
var nodeCount = 0;
var linkCount = 0;
//d3 variable

var linkedByIndex = {};
var width = 950;
var height = 650;
var svg;
var nodes;
var simulation;
var charge_force;
var link_force;
var center_force;
var drag_handler;
var g;
var links;
var nodeSizeScale;
var codeLineClass = 'node-code-line';
let envContent = new Map();
let auxMap = new Map();
let envLineBDepth = new Map();
let lastLineGP = 0;
$(function() {
	//load function
	buildNodes();

	generateSVGgraph();
});

$(document).ready(function() {});

String.prototype.format = function() {
	var i = 0,
		args = arguments;
	return this.replace(/{}/g, function() {
		return typeof args[i] != 'undefined' ? args[i++] : '';
	});
};

function buildNodes() {
	var len = Object.keys(events).length;
	if (len == 0) return;
	eventsLen = len;
	envContent = new Map();
	envLineBDepth = new Map();
	processEnv('main', events[1]['atEnv'], 1, 0);
}

function resolveEnvContents(env, funcName, calledFromLine) {
	let nodeHtml = startBlock();
	nodeHtml += mkBlockHeader(funcName, currentEnv, stackSize == 1 ? 0 : calledFromLine);
	nodeHtml += '<hr>';
	nodeHtml += '<div class="container-fluid" id="node_content">';
	let envBranchDepthInfo = envLineBDepth.get(env);
	for ([ line, content ] of envContent.get(env)) {
		var numEntries = content.length - 1;
		var i;
		let auxHtml = '';
		for (i = numEntries; i >= 0; i--) {
			auxHtml += '{} {} '.format(content[i], i - 1 >= 0 ? '<-' : '');
		}
		let branDepth = envBranchDepthInfo.get(line)[0];
		nodeHtml += genNodeRow(line, auxHtml, branDepth);
	}

	nodeHtml += '</div>';
	nodeHtml += '</node>';
	addNodeToDict(env, nodeHtml);
}

function addNodeToDict(env, html) {
	graph.nodes.push({ name: env, id: ++nodeCount, html: html });
}

function initEnvMap(env) {
	envContent.set(env, new Map());
	envLineBDepth.set(env, new Map());
}

function updateBranchLineDepth(env, line, branchDepth) {
	if (!envLineBDepth.get(env).has(line)) envLineBDepth.get(env).set(line, [ 0 ]);

	envLineBDepth.get(env).get(line)[0] = branchDepth;
}

function processEnv(funcName, env, idxStart, calledFromLine) {
	let i;
	pushToStack(env);
	initEnvMap(env);

	for (i = idxStart; i <= eventsLen; i++) {
		if (events[i]['atEnv'] == env) {
			updateBranchLineDepth(env, events[i]['line'], events[i]['branchDepth']);
			let htmlRcvd = getEventTypeHtml(events[i], i + 1);
			if (
				events[i]['type'] != types.IF &&
				events[i]['type'] != types.FOR &&
				events[i]['type'] != types.REPEAT &&
				events[i]['type'] != types.WHILE
			) {
				lastLineGP = events[i]['line'];
				addEventToEnvMap(env, lastLineGP, htmlRcvd);
			} else {
				i = htmlRcvd;
			}
		}
	}

	resolveEnvContents(env, funcName, calledFromLine);
	popFromStack();
}
function mkBlockHeader(funcName, env, withFromLine) {
	let htmlProduced = '<div class="container-fluid" id="node_header">';

	//Funtion name
	htmlProduced += '<div class="row">';
	htmlProduced += '<div class="col col-5">';
	htmlProduced += '<label class="float-right object-rhs-special">Function:</label>';
	htmlProduced += '</div>';
	htmlProduced += '<div class="col col-6">';
	htmlProduced += '<label>' + funcName.trim() + '()</label>';
	htmlProduced += '</div>';
	htmlProduced += '</div>';

	//environment

	htmlProduced += '<div class="row">';
	htmlProduced += '<div class="col col-5">';
	htmlProduced += '<label class="float-right object-rhs-special">Environment:</label>';
	htmlProduced += '</div>';
	htmlProduced += '<div class="col col-6">';
	htmlProduced += '<label>' + env + '</label>';
	htmlProduced += '</div>';
	htmlProduced += '</div>';
	//from line

	if (withFromLine) {
		//
		htmlProduced += '<div class="row">';
		htmlProduced += '<div class="col col-5">';
		htmlProduced += '<label class="float-right object-rhs-special">From Line:</label>';
		htmlProduced += '</div>';
		htmlProduced += '<div class="col col-6">';
		htmlProduced += '<label>' + withFromLine + '</label>';
		htmlProduced += '</div>';
		htmlProduced += '</div>';
	}

	//terminators
	htmlProduced += '</div>';

	return htmlProduced;
}
function genNodeRow(line, codeHtml, branchDept) {
	var htmlProduced = '<div class="row">';

	htmlProduced += '<div class="col col-lg-2 col1">';
	htmlProduced += '<label class="float-right object-rhs-special">' + line + '</label>';
	htmlProduced += '</div>';
	//code
	htmlProduced += '<div class="col col-md-auto col3" style="margin-left:{}px;">'.format(branchDept * 14);
	htmlProduced += '<codeWrapper">{}</codeWrapper>'.format(codeHtml);
	htmlProduced += '</div>';

	//terminators
	htmlProduced += '</div>';

	return htmlProduced;
}

function addEventToEnvMap(env, line, eventContent) {
	if (!envContent.get(env).has(line)) envContent.get(env).set(line, []);

	if (eventContent.length == 0) return;
	envContent.get(env).get(line).push(eventContent);
}
function genLabelHtml(id, text, indent) {
	return "<label type='button' id='{}' onclick='processEventClick(this.id)' style='margin-left:{}px' data-toggle='modal' data-target='#exec_flow_modal'>{}</label>".format(
		id,
		indent * 14,
		text
	);
}
function genLabelHtmlWithIndent(id, text, indent) {
	return "<label type='button' id='{}' onclick='processEventClick(this.id)' data-toggle='modal' style='margin-left:{}px;' data-target='#exec_flow_modal'>{}{}</label>".format(
		id,
		indent * 14,
		text
	);
}
function genForMultiLabelsWithIdentAndColor(id, text, indent, result) {
	return "<label type='button' id='{}' onclick='processEventClick(this.id)' data-toggle='modal' style='margin-left:{}px;display:block;color:{};' data-target='#exec_flow_modal'>{}{}</label>".format(
		id,
		indent * 14,
		result == true ? 'lightgreen' : 'tomato',
		text
	);
}

function genForMultiLabelsWithIdent(id, text, indent) {
	return "<label type='button' id='{}' onclick='processEventClick(this.id)' data-toggle='modal' style='margin-left:{}px;display:block;' data-target='#exec_flow_modal'>{}{}</label>".format(
		id,
		indent * 14,
		text
	);
}
function genLabelForBigDataAlreadyOpenModal(text) {
	return '<a href="#" id="bd" onclick="processEventClick(this.id)">{}</a>'.format(text);
}
function genLabelForAlreadyOpenModal(id, text) {
	return '<a href="#" id="eId-{}" onclick="processEventClick(this.id)">{}</a>'.format(id, text);
}

function genLabelForAlreadyOpenModalWithIndent(id, text, indent) {
	return '<label id="eId-{}" class="withCursor" style="margin-left:{}px;" onclick="processEventClick(this.id)">{}</label>'.format(
		id,
		indent * 14,
		text
	);
}

function genForMultiLabelsWithIdentAndColorAlreadyOpen(id, text, indent, result) {
	return '<label href="#" id="eId-{}" class="withCursor" style="margin-left:{}px;display:block;color:{};" onclick="processEventClick(this.id)">{}</label>'.format(
		id,
		indent * 14,
		result == true ? 'lightgreen' : 'tomato',
		text
	);
}
function genLabelForAlreadyOpenModalWithIndentAndColor(id, text, indent, result) {
	return '<label href="#" id="eId-{}" class="withCursor" style="margin-left:{}px;color:{};" onclick="processEventClick(this.id)">{}</label>'.format(
		id,
		indent * 14,
		result == true ? 'lightgreen' : 'tomato',
		text
	);
}
function findElseLine(statement, fromLine, isElseIf) {
	let i = fromLine + 1;
	statement = statement.replace(/ /g, '');
	let statementLen = statement.length + 9;
	for (; i < code.length; i++) {
		if (code[i].trim().indexOf(statement) != -1 && (isElseIf ? code[i].trim().length >= statementLen : true)) {
			return i + 1;
		}
	}
	return fromLine;
}

function getEventTypeHtml(event, nextEventId) {
	let line = event['line'];
	let codeLine = code[line - 1];
	let htmlProduced = '';
	switch (event['type']) {
		case types.FUNC:
			let newFuncName = getCodeFlowObjNameById(event['data']['toId']);
			processEnv(newFuncName, event['data']['toEnv'], nextEventId, line);
			addNodeLink(event['atEnv'], event['data']['toEnv']);

			htmlProduced += genLabelHtml(
				'eId-{}'.format(nextEventId - 1),
				codeLine
					.substring(codeLine.indexOf(newFuncName), codeLine.indexOf(')', codeLine.indexOf(newFuncName)) + 1)
					.trim(),
				0
			);

			break;
		case types.DATAF:
			htmlProduced += genLabelHtml(
				'eId-{}'.format(nextEventId - 1),
				codeLine.substring(codeLine.indexOf('data.frame')).trim(),
				0
			);
			break;
		case types.ASSIGN:
			let objId = event['data']['toObj'];
			let obj = getCommonObjNameById(objId);
			let state = event['data']['toState'];
			let origin = event['data']['origin'];
			if (origin == 'obj') {
				htmlProduced += genLabelHtml('eId-{}'.format(nextEventId - 1), codeLine.trim(), event['branchDepth']);
			} else {
				if (origin == 'event' && events[event['data']['fromEvent']]['type'] == types.VEC) {
					// the vector creation do not have anything special worth a separated label
					htmlProduced += genLabelHtml('eId-{}'.format(nextEventId - 1, objId, state), codeLine.trim(), 0);
				} else {
					//other event types
					htmlProduced += genLabelHtml(
						'eId-{}'.format(nextEventId - 1, objId, state),
						codeLine.substring(codeLine.indexOf(obj), codeLine.indexOf('<-', codeLine.indexOf(obj))).trim(),
						0
					);
				}
			}
			break;
		case types.IF:
			let env = event['atEnv'];
			let startLine = event['line'];
			let eventId = nextEventId - 1;
			let statement;
			while (event['type'] == types.IF && event['line'] == startLine) {
				//collect all if statements below
				if (event['data']['isElseIf']) {
					statement = 'else if ( {} )'.format(event['data']['exprStr']);
				} else if (event['data']['isElse']) {
					statement = 'else';
				} else {
					statement = 'if ( {} )'.format(event['data']['exprStr']);
				}
				htmlProduced += genForMultiLabelsWithIdentAndColor(
					'eId-{}'.format(eventId),
					statement,
					0,
					event['data']['globalResult']
				);

				//pick next event
				event = events[++eventId];
			}
			addEventToEnvMap(env, startLine, htmlProduced);
			return eventId - 1;

			break;
		case types.RET:
			if (typeof envContent.get(event['atEnv']).get(line) === 'undefined') {
				//no content for this line, so, grab the code existing there
				htmlProduced += "<ret type='button' data-toggle='modal' data-target='#exec_flow_modal' id='eId-{}' onclick='processEventClick(this.id)'> {} <- {} </ret>".format(
					nextEventId - 1,
					'(return)',
					codeLine
				);
			} else {
				//already have something for this line, just append this, resolver takes care
				htmlProduced += "<ret type='button' data-toggle='modal' data-target='#exec_flow_modal' id='eId-{}' onclick='processEventClick(this.id)'> {} </ret>".format(
					nextEventId - 1,
					'(return)'
				);
			}
			break;
		case types.ARITH:
			htmlProduced += genLabelHtml('eId-{}'.format(nextEventId - 1), event['data']['exprStr'].trim(), 0);
			break;
		case types.IDX:
			let originIdx = event['data']['origin'];
			let toId = event['data']['toId'];
			let toState = event['data']['toState'];
			let objIdx = getCommonObjById(toId);
			if (originIdx == 'obj') {
				htmlProduced += genLabelHtml('eId-{}'.format(nextEventId - 1), codeLine.trim(), event['branchDepth']);
			} else {
				if (originIdx == 'event' && events[event['data']['fromEvent']]['type'] == types.VEC) {
					// the vector creation do not have anything special worth a separated label
					htmlProduced += genLabelHtml('eId-{}'.format(nextEventId - 1, toId, toState), codeLine.trim(), 0);
				} else {
					//other event types
					htmlProduced += genLabelHtml(
						'eId-{}'.format(nextEventId - 1, toId, toState),
						codeLine
							.substring(codeLine.indexOf(objIdx), codeLine.indexOf('<-', codeLine.indexOf(objIdx)))
							.trim(),
						0
					);
				}
			}
			break;
		case types.FOR: {
			let envir = event['atEnv'];
			if (codeLine.includes('{')) codeLine = codeLine.substring(0, codeLine.indexOf('{'));
			htmlProduced += genLabelHtml('eId-{}'.format(nextEventId - 1), codeLine.trim(), event['branchDepth']);
			let i;
			let lastEventId = event['data']['lastEventId'];
			for (i = nextEventId; i != lastEventId; i++) {
				if (events[i]['type'] == types.FUNC) {
					let newFuncName = getCodeFlowObjNameById(events[i]['data']['toId']);
					processEnv(newFuncName, events[i]['data']['toEnv'], nextEventId, line);
					addNodeLink(events[i]['atEnv'], events[i]['data']['toEnv']);
				}
			}
			addEventToEnvMap(envir, event['line'], htmlProduced);
			return lastEventId;
		}
		case types.REPEAT: {
			let envir = event['atEnv'];
			if (codeLine.includes('{')) codeLine = codeLine.substring(0, codeLine.indexOf('{'));
			htmlProduced += genLabelHtml('eId-{}'.format(nextEventId - 1), codeLine.trim(), event['branchDepth']);
			let i;
			let lastEventId = event['data']['lastEventId'];
			for (i = nextEventId; i != lastEventId; i++) {
				if (events[i]['type'] == types.FUNC) {
					let newFuncName = getCodeFlowObjNameById(events[i]['data']['toId']);
					processEnv(newFuncName, events[i]['data']['toEnv'], nextEventId, line);
					addNodeLink(events[i]['atEnv'], events[i]['data']['toEnv']);
				}
			}
			addEventToEnvMap(envir, event['line'], htmlProduced);
			return lastEventId;
		}
		case types.WHILE: {
			let envir = event['atEnv'];
			if (codeLine.includes('{')) codeLine = codeLine.substring(0, codeLine.indexOf('{'));
			htmlProduced += genLabelHtml('eId-{}'.format(nextEventId - 1), codeLine.trim(), event['branchDepth']);
			let i;
			let lastEventId = event['data']['lastEventId'];
			for (i = nextEventId; i != lastEventId; i++) {
				if (events[i]['type'] == types.FUNC) {
					let newFuncName = getCodeFlowObjNameById(events[i]['data']['toId']);
					processEnv(newFuncName, events[i]['data']['toEnv'], nextEventId, line);
					addNodeLink(events[i]['atEnv'], events[i]['data']['toEnv']);
				}
			}
			addEventToEnvMap(envir, event['line'], htmlProduced);
			return lastEventId;
		}
		default:
			break;
	}
	return htmlProduced;
}

function addNodeLink(sourceEnv, targetEnv) {
	graph.links.push({ source: sourceEnv, target: targetEnv, id: ++linkCount });
}

function startBlock() {
	if (stackSize == 1) {
		return '<node class="container-fluid overflow-visible main-node" id="env-{}">'.format(stack[stackSize - 1]);
	} else {
		return '<node class="container-fluid overflow-visible" id="env-{}">'.format(stack[stackSize - 1]);
	}
}

function pushToStack(env) {
	stack.push(env);
	currentEnv = env;
	stackSize++;
}

function popFromStack() {
	stack.pop();
	stackSize--;
	currentEnv = stack[stackSize - 1];
}

function mkTooltip(objCurrentValues) {
	return "<a href='#' type='button' data-placement='right' data-toggle='tooltip' data-html='true' title='Size: {}</br>{}'><u>values!</u></a>".format(
		objCurrentValues[3].length,
		objCurrentValues[3]
	);
}

function mkTooltip2(firstElement, secondElement, text) {
	return "<a href='#' type='button' data-placement='top' data-toggle='tooltip' data-html='true' title='{}: {}</br>{}: {}'><u>{}</u></a>".format(
		firstElement[0],
		firstElement[1],
		secondElement[0],
		secondElement[1],
		text
	);
}

function mkTooltipOneLine(element, text) {
	return "<a href='#' type='button' data-placement='top' data-toggle='tooltip' data-html='true' title='{}: {}'><u>{}</u></a>".format(
		element[0],
		element[1],
		text
	);
}

function mkFuncModalInfo(event, eventId) {
	let htmlProduced = '';
	let args = event['data']['args'];
	let targetFunc = event['data']['toId'];
	//container open
	htmlProduced += '<div class="container-fluid">';

	//header
	htmlProduced += '<div class="row">';
	htmlProduced += '<div class="col-9 text-left dialog-title">Function Name: {}()'.format(
		getCodeFlowObjNameById(targetFunc)
	);
	htmlProduced += '</div>';
	htmlProduced += '</div>';

	htmlProduced += '<div class="row">';
	htmlProduced += '<div class="col-9 text-left dialog-title mt-5">Arguments';
	htmlProduced += '</div>';
	htmlProduced += '</div>';
	//contents (table)
	htmlProduced += '<table class="table table-sm mt-4">';

	//table head
	htmlProduced += '<thead>';
	htmlProduced += '<th class ="text-center" scope="col">Passed</th>';
	htmlProduced += '<th class ="text-center" scope="col">Received</th>';
	htmlProduced += '<th class ="text-center" scope="col">Value</th>';
	htmlProduced += '<th class ="text-center" scope="col">Previous change</th>';
	htmlProduced += '</tr>';
	htmlProduced += '</thead>';
	//table body
	htmlProduced += '<tbody class="text-center">';
	if (args.length > 0) {
		//process args
		args.forEach((arg) => {
			let fromId = arg['fromId'];
			let fromState = arg['fromState'];
			let toId = arg['toId'];
			let fromObj = arg['fromId'] > 0 ? getCommonObjNameById(fromId) : fromId == -1 ? 'U-T' : 'N-T';
			let toObj = getCommonObjNameById(toId);
			let objCurrentValues = getObjCurrValue(toId, 1, -1);
			let prevChange = 'NA';

			if (fromId > 0) {
				let foundEvent = findEventId(fromId, fromState);

				prevChange = genLabelForAlreadyOpenModal(foundEvent, events[foundEvent]['line']);
			}

			htmlProduced += '<tr>';
			htmlProduced += '<td>{}</td>'.format(fromObj);
			htmlProduced += '<td>{}</td>'.format(toObj);
			htmlProduced += '<td>{}</td>'.format(structToStr(objCurrentValues));
			htmlProduced += '<td>{}</td>'.format(prevChange);
			htmlProduced += '</tr>';
		});

		htmlProduced +=
			'<caption style="font-size: 11pt;"> <p>U-T : User-Typed ; N-T : Not-Tracked ; NA - Not Availabel</p></caption>';
	} else {
		//no args
		htmlProduced += '<tr>';
		htmlProduced += '<td colspan="4" class="text-center">No arguments to display!</td>';
		htmlProduced += '</tr>';
	}

	htmlProduced += '</tbody>';
	htmlProduced += '</table>';

	//container close
	htmlProduced += '</div>';
	return htmlProduced;
}
function mkObjModalTopInfo(event) {
	let htmlProduced = '';
	let line = event['line'];
	let objId = event['data']['toObj'];
	let objState = event['data']['toState'];
	let objCurrentValues = getObjCurrValue(objId, objState, -1);
	let sourceEvent = '';
	if (event['data']['origin'] == 'event') {
		switch (events[event['data']['fromEvent']]['type']) {
			case types.DATAF:
				sourceEvent = genLabelForAlreadyOpenModal(event['data']['fromEvent'], 'DataFrame Creation');
				break;
			case types.VEC:
				sourceEvent = 'Vector Creation';
				break;
			case types.RET:
				sourceEvent = '{}() return'.format(
					getCodeFlowObjNameById(events[event['data']['fromEvent']]['atFunc'])
				);
				break;
			case types.ARITH:
				sourceEvent = genLabelForAlreadyOpenModal(event['data']['fromEvent'], 'Arithmetic');
				break;
		}
	} else {
		if (event['data']['fromObj'] == 'ABD') {
			let eventFound = findEventId(event['data']['fromId'], event['data']['fromState']);
			let srcObj = getCommonObjNameById(event['data']['fromId']);
			sourceEvent = '<label id="eId-{}" onclick="processEventClick(this.id)"><a href="#">{}</a></label>'.format(
				eventFound,
				srcObj
			);
		} else {
			sourceEvent = 'User-Typed';
		}
	}
	htmlProduced += '<div class="container-fluid">';

	//first section header
	htmlProduced += '<div class="row">';
	htmlProduced += '<div class="col-9 text-left dialog-title"> Current Status (Line: {})'.format(line);
	htmlProduced += '</div>';
	htmlProduced += '</div>';
	//first section obj name info
	htmlProduced += '<div class="row mt-3">';
	htmlProduced += '<div class="col text-right">Name:';
	htmlProduced += '</div>';
	htmlProduced += '<div class="col text-left">{}'.format(getCommonObjNameById(objId));
	htmlProduced += '</div>';
	htmlProduced += '</div>';
	//first section obj structure Type
	htmlProduced += '<div class="row">';
	htmlProduced += '<div class="col text-right">Structure:';
	htmlProduced += '</div>';
	htmlProduced += '<div class="col text-left">{}'.format(objCurrentValues[0]);
	htmlProduced += '</div>';
	htmlProduced += '</div>';
	//first section obj data Type
	htmlProduced += '<div class="row">';
	htmlProduced += '<div class="col text-right">Data type:';
	htmlProduced += '</div>';
	htmlProduced += '<div class="col text-left">{}'.format(objCurrentValues[1]);
	htmlProduced += '</div>';
	htmlProduced += '</div>';
	//New Value source
	htmlProduced += '<div class="row">';
	htmlProduced += '<div class="col text-right">Value from:';
	htmlProduced += '</div>';

	htmlProduced += '<div class="col text-left">{}'.format(sourceEvent);
	htmlProduced += '</div>';
	htmlProduced += '</div>';
	//first section obj size
	htmlProduced += '<div class="row">';
	htmlProduced += '<div class="col text-right">New value size:';
	htmlProduced += '</div>';
	htmlProduced += '<div class="col text-left">{}'.format(objCurrentValues[2]);
	htmlProduced += '</div>';
	htmlProduced += '</div>';
	//first section obj value
	htmlProduced += '<div class="row">';
	htmlProduced += '<div class="col text-right">New value:';
	htmlProduced += '</div>';
	htmlProduced += '<div class="col text-left">';

	htmlProduced += structToStr(objCurrentValues);

	htmlProduced += '</div>';
	htmlProduced += '</div>';

	return htmlProduced;
}
let toSearch = [];
let search = 0;
function mkObjModalBotInfo(event, eventId) {
	let htmlProduced = '';

	//second section header
	htmlProduced += '<div class="row mt-5">';
	htmlProduced += '<div class="col-9 text-left dialog-title">History';
	htmlProduced += '</div>';
	htmlProduced += '</div>';

	//second section table start
	htmlProduced += '<table class="table table-sm mt-2">';
	htmlProduced += '<thead>';
	//table headers
	htmlProduced += '<tr class="dialog-text">';
	htmlProduced += '<th class ="text-center" scope="col">Line</th>';
	htmlProduced += '<th class ="text-center" scope="col">Executed Code</th>';
	htmlProduced += '<th class ="text-center" scope="col">Values</th>';
	htmlProduced += '</tr>';
	htmlProduced += '</thead>';
	htmlProduced += '<tbody class="text-center">';
	//table rows

	let currentEnv = event['atEnv'];
	let i;
	let objId = event['data']['toObj'];
	let obj = getCommonObjById(objId);
	let doneSomething = false;
	if (obj['usages'] > 1 && eventId - 1 > 1) {
		for (i = 1; i < eventId; i++) {
			let currEvent = events[i];
			if (currEvent['atEnv'] == currentEnv && currEvent['type'] == 'assign_event') {
				if (currEvent['data']['toObj'] == objId) {
					doneSomething = true;
					let objStateValues = getObjCurrValue(objId, currEvent['data']['toState'], -1);
					let strToAppend = '';
					//create row here
					htmlProduced += '<tr>';
					htmlProduced += '<td class ="text-center" id="eId-{}" onclick="processEventClick(this.id)"><a href="#">{}</a></td>'.format(
						i,
						currEvent['line']
					);
					if (currEvent['data']['fromObj'] == 'ABD') {
						//the value came from another object
						let fromId = currEvent['data']['fromId'];
						let fromState = currEvent['data']['fromState'];
						addToSearch(fromId, fromState, 'hist-' + (currEvent['line'] - 1));
					}

					htmlProduced += '<td class="text-center" id="hist-{}">{}</td>'.format(
						currEvent['line'] - 1,
						code[currEvent['line'] - 1]
					);

					htmlProduced += '<td class ="text-center">{}</td>'.format(structToStr(objStateValues));
					htmlProduced += '</tr>';
				}
			}
		}
	}
	if (!doneSomething) {
		//no records to show
		htmlProduced += '<tr>';
		htmlProduced += '<td colspan="3" class="text-center" >No records to display!</td>';
		htmlProduced += '</tr>';
	}

	//second section table finish
	htmlProduced += '</tbody>';
	htmlProduced += '</table>';

	//container close
	htmlProduced += '</div>';
	return htmlProduced;
}
function addToSearch(fromId, fromState, tdId) {
	toSearch.push({
		fromId: fromId,
		fromState: fromState,
		tdId: tdId
	});
	search++;
}
function findEventId(toObj, toState) {
	let i;
	for (i = 1; i <= Object.keys(events).length; i++) {
		if (events[i]['type'] == types.ASSIGN) {
			if (events[i]['data']['toObj'] == toObj && events[i]['data']['toState'] == toState) return i;
		}
	}
	return 0;
}
function doSearch() {
	toSearch.forEach((searchable) => {
		let eventId = 0;
		if ((eventId = findEventId(searchable.fromId, searchable.fromState))) {
			let lineNum = searchable.tdId.split('-')[1];
			let codeLine = code[lineNum].split('<-');
			let aTag = genLabelForAlreadyOpenModal(eventId, codeLine[1]);
			let html = '{} <- {}'.format(codeLine[0], aTag);
			document.getElementById(searchable.tdId).innerHTML = html;
		}
	});
	search = 0;
	toSearch = [];
}

function getHtmlForExpressions(event, showLogical) {
	let nExpr = Object.keys(event['data']['expressions']).length;
	let exprs = event['data']['expressions'];
	let i;
	let htmlProduced = '';

	for (i = 1; i <= nExpr; i++) {
		let cE = exprs[i];
		htmlProduced += '<tr>';
		htmlProduced += '<td>{}</td>'.format(i);
		//treat left object
		htmlProduced += '<td>';

		if (cE['lType'] == 'expr') {
			// left is expr
			let element = [];
			if (showLogical) element = [ 'Result', exprs[cE['lExpId']]['result'] != 0 ? 'true' : 'false' ];
			else element = [ 'Result', exprs[cE['lExpId']]['result'] ];

			let text = '#{}'.format(cE['lExpId']);
			htmlProduced += mkTooltipOneLine(element, text);
		} else {
			switch (cE['lObjId']) {
				case -1:
					// HARDCODED (just put the value there)
					htmlProduced += cE['lValue'];
					break;
				case -2:
					// NON-TRACKED
					let firstElement = [ 'Obj', 'Non-tracked' ];
					let secondElement = [ 'Value', cE['lValue'] ];
					let text = '{}[{}]'.format(cE['lObjName'], cE['lWithIndex'] + 1);

					htmlProduced += mkTooltip2(firstElement, secondElement, text);
					break;
				default:
					//ABD_OBJECT
					let objValues = getObjCurrValue(cE['lObjId'], cE['lObjState'], cE['lWithIndex']);
					let stateEventId = findEventId(cE['lObjId'], cE['lObjState']);
					let label = genLabelForAlreadyOpenModal(
						stateEventId,
						'{}[{}]'.format(getCommonObjNameById(cE['lObjId']), cE['lWithIndex'] + 1)
					);
					let element = [ 'Value', objValues[3] ];
					htmlProduced += mkTooltipOneLine(element, label);

					break;
			}
		}

		htmlProduced += '</td>';

		//operator
		htmlProduced += '<td>{}</td>'.format(cE['op']);

		//treat rightObj
		htmlProduced += '<td>';
		if (cE['rType'] == 'expr') {
			// left is expr
			let element = [];
			if (showLogical) element = [ 'Result', exprs[cE['rExpId']]['result'] != 0 ? 'true' : 'false' ];
			else element = [ 'Result', exprs[cE['rExpId']]['result'] ];
			let text = '#{}'.format(cE['rExpId']);
			htmlProduced += mkTooltipOneLine(element, text);
		} else {
			switch (cE['rObjId']) {
				case -1:
					// HARDCODED (just put the value there)
					htmlProduced += cE['rValue'];
					break;
				case -2:
					// NON-TRACKED
					let firstElement = [ 'Obj', 'Non-tracked' ];
					let secondElement = [ 'Value', cE['rValue'] ];
					let text = '{}[{}]'.format(cE['rObjName'], cE['rWithIndex'] + 1);

					htmlProduced += mkTooltip2(firstElement, secondElement, text);
					break;
				default:
					//ABD_OBJECT
					let objValues = getObjCurrValue(cE['rObjId'], cE['rObjState'], cE['rWithIndex']);
					let stateEventId = findEventId(cE['rObjId'], cE['rObjState']);
					let label = genLabelForAlreadyOpenModal(
						stateEventId,
						'{}[{}]'.format(getCommonObjNameById(cE['rObjId']), cE['rWithIndex'] + 1)
					);

					let element = [ 'Value', objValues[3] ];
					htmlProduced += mkTooltipOneLine(element, label);
					break;
			}
		}
		htmlProduced += '</td>';
		//result
		if (showLogical) htmlProduced += '<td>{} ({})</td>'.format(cE['result'], cE['result'] != 0 ? 'T' : 'F');
		else htmlProduced += '<td>{}</td>'.format(cE['result']);

		htmlProduced += '</tr>';
	}

	return htmlProduced;
}

function findFailedTests(eventId) {
	let i = eventId - 1;
	let failedExpressions = [];

	for (; i > 0; i--) {
		if (events[i]['type'] == 'if_event') {
			failedExpressions.push({
				id: i,
				expr: events[i]['data']['exprStr'],
				result: events[i]['data']['globalResult']
			});
			if (!events[i]['data']['isElseIf']) {
				return failedExpressions;
			}
		}
	}
}
function mkFailedTestsHtml(failedTests) {
	let htmlProduced = '';
	htmlProduced += '<div class="row mt-5">';
	htmlProduced += '<div class="col-9 text-left dialog-title2">Current branch failed conditions';
	htmlProduced += '</div>';
	htmlProduced += '</div>';
	htmlProduced += '<table class="table table-sm mt-2">';
	htmlProduced += '<thead>';
	//table headers
	htmlProduced += '<tr class="dialog-text">';
	htmlProduced += '<th class ="text-center" scope="col">Event</th>';
	htmlProduced += '<th class ="text-center" scope="col">Expression</th>';
	htmlProduced += '<th class ="text-center" scope="col">Result</th>';
	htmlProduced += '</tr>';
	htmlProduced += '</thead>';
	htmlProduced += '<tbody class="text-center">';
	failedTests.forEach((test) => {
		htmlProduced += '<tr>';
		let lblJump = genLabelForAlreadyOpenModal(test.id, 'View');

		htmlProduced += '<td>{}</td>'.format(lblJump);
		htmlProduced += '<td>{}</td>'.format(test.expr);
		htmlProduced += '<td>{}</td>'.format(test.result);
		htmlProduced += '</tr>';
	});
	htmlProduced += '</tbody>';
	htmlProduced += '</table>';

	return htmlProduced;
}
function mkArithModalInfo(event, eventId) {
	let htmlProduced = '';
	htmlProduced += '<div class="container-fluid">';

	htmlProduced += '<div class="row mt-2">';
	htmlProduced += '<div class="col text-left">Condition:';
	htmlProduced += '</div>';
	htmlProduced += '<div class="col-md-auto text-left">{}'.format(event['data']['exprStr']);
	htmlProduced += '</div>';
	htmlProduced += '</div>';
	//first section obj structure Type
	htmlProduced += '<div class="row">';
	htmlProduced += '<div class="col text-left">Result:';
	htmlProduced += '</div>';
	htmlProduced += '<div class="col-md-auto text-left">{}'.format(event['data']['result']);
	htmlProduced += '</div>';
	htmlProduced += '</div>';

	htmlProduced += '<div class="row mt-5">';
	htmlProduced += '<div class="col-9 text-left dialog-title">Operations';
	htmlProduced += '</div>';
	htmlProduced += '</div>';

	//second section table start
	htmlProduced += '<table class="table table-sm mt-2">';
	htmlProduced += '<thead>';
	//table headers
	htmlProduced += '<tr class="dialog-text">';
	htmlProduced += '<th class ="text-center" scope="col">ID</th>';
	htmlProduced += '<th class ="text-center" scope="col">L-Operand</th>';
	htmlProduced += '<th class ="text-center" scope="col">Operator</th>';
	htmlProduced += '<th class ="text-center" scope="col">R-Operand</th>';
	htmlProduced += '<th class ="text-center" scope="col">Result</th>';
	htmlProduced += '</tr>';
	htmlProduced += '</thead>';
	htmlProduced += '<tbody class="text-center">';
	htmlProduced += getHtmlForExpressions(event, false);
	htmlProduced += '</tbody>';
	htmlProduced += '</table>';
	htmlProduced += '</div>';

	return htmlProduced;
}
function mkIfModalInfo(event, eventId) {
	//
	let htmlProduced = '';
	htmlProduced += '<div class="container-fluid">';

	htmlProduced += '<div class="row mt-2">';
	htmlProduced += '<div class="col text-left">Condition:';
	htmlProduced += '</div>';
	htmlProduced += '<div class="col-md-auto text-left">{}'.format(event['data']['exprStr']);
	htmlProduced += '</div>';
	htmlProduced += '</div>';
	//first section obj structure Type
	htmlProduced += '<div class="row">';
	htmlProduced += '<div class="col text-left">Result:';
	htmlProduced += '</div>';
	htmlProduced += '<div class="col-md-auto text-left">{}'.format(event['data']['globalResult']);
	htmlProduced += '</div>';
	htmlProduced += '</div>';

	htmlProduced += '<div class="row mt-5">';
	htmlProduced += '<div class="col-9 text-left dialog-title2">Condition breakdown';
	htmlProduced += '</div>';
	htmlProduced += '</div>';

	//second section table start
	htmlProduced += '<table class="table table-sm mt-2">';
	htmlProduced += '<thead>';
	//table headers
	htmlProduced += '<tr class="dialog-text">';
	htmlProduced += '<th class ="text-center" scope="col">ID</th>';
	htmlProduced += '<th class ="text-center" scope="col">L-Operand</th>';
	htmlProduced += '<th class ="text-center" scope="col">Operator</th>';
	htmlProduced += '<th class ="text-center" scope="col">R-Operand</th>';
	htmlProduced += '<th class ="text-center" scope="col">Result</th>';
	htmlProduced += '</tr>';
	htmlProduced += '</thead>';
	htmlProduced += '<tbody class="text-center">';
	if (event['data']['isElse']) {
		//is else
		//need to find previous events (until isElseIf == false, inclusive)
		htmlProduced += '<tr> <td colspan="5" class="text-center"> No conditions to display </td></tr>';
		htmlProduced += '</tbody>';
		htmlProduced += '</table>';
		let failedTests = findFailedTests(eventId).reverse();
		htmlProduced += mkFailedTestsHtml(failedTests);
	} else {
		if (event['data']['isElseIf']) {
			// else if statement
			htmlProduced += getHtmlForExpressions(event, true);
			htmlProduced += '</tbody>';
			htmlProduced += '</table>';
			let failedTests = findFailedTests(eventId).reverse();
			htmlProduced += mkFailedTestsHtml(failedTests);
		} else {
			// normal if statement
			htmlProduced += getHtmlForExpressions(event, true);
			htmlProduced += '</tbody>';
			htmlProduced += '</table>';
		}
	}

	htmlProduced += '</div>';
	return htmlProduced;
}

function mkForLoopModalInfo(event, eventId) {
	let htmlProduced = '';
	let iteratorName = getCommonObjNameById(event['data']['iteratorId']);
	let enumeratorId = event['data']['enumeratorId'];
	let enumeratorObj = getCommonObjById(enumeratorId);
	let enumeratorState = event['data']['enumeratorState'];
	let enumStateEventId = findEventId(enumeratorId, enumeratorState);
	htmlProduced += '<div class="container-fluid">';

	htmlProduced += '<div class="row mt-2">';
	htmlProduced += '<div class="col text-left">Iterator:';
	htmlProduced += '</div>';
	htmlProduced += '<div class="col-md-auto text-left">{}'.format(iteratorName);
	htmlProduced += '</div>';
	htmlProduced += '</div>';
	//first section obj structure Type
	htmlProduced += '<div class="row">';
	htmlProduced += '<div class="col text-left">Enumerator:';
	htmlProduced += '</div>';
	htmlProduced += '<div class="col-md-auto text-left">{}'.format(
		genLabelForAlreadyOpenModal(enumStateEventId, enumeratorObj['name'])
	);
	htmlProduced += '</div>';
	htmlProduced += '</div>';

	htmlProduced += '<div class="row">';
	htmlProduced += '<div class="col text-left">Expected Iterations:';
	htmlProduced += '</div>';
	htmlProduced += '<div class="col-md-auto text-left">{}'.format(event['data']['estimatedIter']);
	htmlProduced += '</div>';
	htmlProduced += '</div>';

	htmlProduced += '<div class="row">';
	htmlProduced += '<div class="col text-left">Effective Iterations:';
	htmlProduced += '</div>';

	htmlProduced += '<div class="col-md-auto text-left">{}'.format(event['data']['iterCounter']);
	htmlProduced += '</div>';
	htmlProduced += '</div>';

	htmlProduced += '<div class="row mt-4">';
	htmlProduced += '<div class="col text-center">Previous';
	htmlProduced += '</div>';
	htmlProduced += '<div class="col text-center">Iteration';
	htmlProduced += '</div>';
	htmlProduced += '<div class="col text-center">Next';
	htmlProduced += '</div>';
	htmlProduced += '</div>';
	htmlProduced += '<div class="row">';

	let dropDown = '';
	dropDown += '<div class="dropdown show">';
	dropDown +=
		'<a class="btn btn-info btn-sm dropdown-toggle" href="#" role="button" id="dropdownMenuLink" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false">';
	dropDown += '1';
	dropDown += '</a>';
	dropDown += '<div class="dropdown-menu" aria-labelledby="dropdownMenuLink">';
	dropDown += '<input class="form-control" id="dropSearch-{}" type="text" placeholder="Search...">'.format(eventId);
	let i;
	for (i = 1; i <= event['data']['iterCounter']; i++)
		dropDown += '<a id="itId-{}" class="dropdown-item text-left" href="#" onclick="processIteration({},this.id, false)">{}</a>'.format(
			i,
			eventId,
			i
		);

	dropDown += '</div>';
	dropDown += '</div>';

	htmlProduced += '<div class="col text-center">{}'.format(
		"<i id='eId-{}' class='fa fa-arrow-left' aria-hidden='true' style='font-size:22px;color:var(--title-color);cursor: pointer;' onclick='requestPrevIteration(this.id)'></i>".format(
			eventId
		)
	);
	htmlProduced += '</div>';
	htmlProduced += '<div class="col text-center">{}'.format(dropDown);
	htmlProduced += '</div>';
	htmlProduced += '<div class="col text-center">{}'.format(
		"<i id='eId-{}'  class='fa fa-arrow-right' aria-hidden='true' style='font-size:22px;color:var(--title-color);cursor: pointer;' onclick='requestNextIteration(this.id)'></i>".format(
			eventId
		)
	);
	htmlProduced += '</div>';
	htmlProduced += '</div>';

	htmlProduced += '<table class="table table-sm mt-4">';
	htmlProduced += '<thead>';
	//table headers
	htmlProduced += '<tr class="dialog-text">';
	htmlProduced +=
		'<th id="for_iteration_body_header" class ="text-center" scope="col" colspan="3">Iteration 1 information</th>';
	htmlProduced += '</tr>';
	htmlProduced += '</thead>';
	htmlProduced += '<tbody class="text-left for-loop-body " id="for_iteration_body">';
	htmlProduced += processIteration(eventId, 1, true);
	htmlProduced += '</tbody>';
	htmlProduced += '</div>';
	return htmlProduced;
}

function mkRepeatLoopModalInfo(event, eventId) {
	let htmlProduced = '';

	htmlProduced += '<div class="container-fluid">';

	htmlProduced += '<div class="row">';
	htmlProduced += '<div class="col text-left">Number of iterations:';
	htmlProduced += '</div>';

	htmlProduced += '<div class="col-md-auto text-left">{}'.format(event['data']['iterCounter']);
	htmlProduced += '</div>';
	htmlProduced += '</div>';

	htmlProduced += '<div class="row mt-4">';
	htmlProduced += '<div class="col text-center">Previous';
	htmlProduced += '</div>';
	htmlProduced += '<div class="col text-center">Iteration';
	htmlProduced += '</div>';
	htmlProduced += '<div class="col text-center">Next';
	htmlProduced += '</div>';
	htmlProduced += '</div>';
	htmlProduced += '<div class="row">';

	let dropDown = '';
	dropDown += '<div class="dropdown show">';
	dropDown +=
		'<a class="btn btn-info btn-sm dropdown-toggle" href="#" role="button" id="dropdownMenuLink" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false">';
	dropDown += '1';
	dropDown += '</a>';
	dropDown += '<div class="dropdown-menu" aria-labelledby="dropdownMenuLink">';
	dropDown += '<input class="form-control" id="dropSearch-{}" type="text" placeholder="Search...">'.format(eventId);
	let i;
	for (i = 1; i <= event['data']['iterCounter']; i++)
		dropDown += '<a id="itId-{}" class="dropdown-item text-left" href="#" onclick="processRepeatIteration({},this.id, false)">{}</a>'.format(
			i,
			eventId,
			i
		);

	dropDown += '</div>';
	dropDown += '</div>';

	htmlProduced += '<div class="col text-center">{}'.format(
		"<i id='eId-{}' class='fas fa-arrow-left' style='font-size:22px;color:var(--title-color);cursor: pointer;' onclick='requestPrevRepeatIteration(this.id)'></i>".format(
			eventId
		)
	);
	htmlProduced += '</div>';
	htmlProduced += '<div class="col text-center">{}'.format(dropDown);
	htmlProduced += '</div>';
	htmlProduced += '<div class="col text-center">{}'.format(
		"<i id='eId-{}'  class='fa fa-arrow-right' aria-hidden='true' style='font-size:22px;color:var(--title-color);cursor: pointer;' onclick='requestNextRepeatIteration(this.id)'></i>".format(
			eventId
		)
	);
	htmlProduced += '</div>';
	htmlProduced += '</div>';

	htmlProduced += '<table class="table table-sm mt-4">';
	htmlProduced += '<thead>';
	//table headers
	htmlProduced += '<tr class="dialog-text">';
	htmlProduced +=
		'<th id="for_iteration_body_header" class ="text-center" scope="col" colspan="3">Iteration 1 information</th>';
	htmlProduced += '</tr>';
	htmlProduced += '</thead>';
	htmlProduced += '<tbody class="text-left for-loop-body " id="for_iteration_body">';
	htmlProduced += processRepeatIteration(eventId, 1, true);
	htmlProduced += '</tbody>';
	htmlProduced += '</div>';
	return htmlProduced;
}

function mkWhileLoopModalInfo(event, eventId) {
	let htmlProduced = '';

	htmlProduced += '<div class="container-fluid">';

	htmlProduced += '<div class="row">';
	htmlProduced += '<div class="col text-left">Number of iterations:';
	htmlProduced += '</div>';

	htmlProduced += '<div class="col-md-auto text-left">{}'.format(event['data']['iterCounter']);
	htmlProduced += '</div>';
	htmlProduced += '</div>';
	htmlProduced += '<div class="row">';
	htmlProduced += '<div class="col text-left">Loop condition:';
	htmlProduced += '</div>';

	htmlProduced += '<div class="col-md-auto text-left">{}'.format(event['data']['statement']);
	htmlProduced += '</div>';
	htmlProduced += '</div>';

	htmlProduced += '<div class="row mt-4">';
	htmlProduced += '<div class="col text-center">Previous';
	htmlProduced += '</div>';
	htmlProduced += '<div class="col text-center">Iteration';
	htmlProduced += '</div>';
	htmlProduced += '<div class="col text-center">Next';
	htmlProduced += '</div>';
	htmlProduced += '</div>';
	htmlProduced += '<div class="row">';

	let dropDown = '';
	dropDown += '<div class="dropdown show">';
	dropDown +=
		'<a class="btn btn-info btn-sm dropdown-toggle" href="#" role="button" id="dropdownMenuLink" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false">';
	dropDown += '1';
	dropDown += '</a>';
	dropDown += '<div class="dropdown-menu" aria-labelledby="dropdownMenuLink">';
	dropDown += '<input class="form-control" id="dropSearch-{}" type="text" placeholder="Search...">'.format(eventId);
	let i;
	for (i = 1; i <= event['data']['iterCounter']; i++)
		dropDown += '<a id="itId-{}" class="dropdown-item text-left" href="#" onclick="processWhileIteration({},this.id, false)">{}</a>'.format(
			i,
			eventId,
			i
		);

	dropDown += '</div>';
	dropDown += '</div>';

	htmlProduced += '<div class="col text-center">{}'.format(
		"<i id='eId-{}' class='fas fa-arrow-left' style='font-size:22px;color:var(--title-color);cursor: pointer;' onclick='requestPrevWhileIteration(this.id)'></i>".format(
			eventId
		)
	);
	htmlProduced += '</div>';
	htmlProduced += '<div class="col text-center">{}'.format(dropDown);
	htmlProduced += '</div>';
	htmlProduced += '<div class="col text-center">{}'.format(
		"<i id='eId-{}'  class='fa fa-arrow-right' aria-hidden='true' style='font-size:22px;color:var(--title-color);cursor: pointer;' onclick='requestNextWhileIteration(this.id)'></i>".format(
			eventId
		)
	);
	htmlProduced += '</div>';
	htmlProduced += '</div>';

	htmlProduced += '<table class="table table-sm mt-4">';
	htmlProduced += '<thead>';
	//table headers
	htmlProduced += '<tr class="dialog-text">';
	htmlProduced +=
		'<th id="for_iteration_body_header" class ="text-center" scope="col" colspan="3">Iteration 1 information</th>';
	htmlProduced += '</tr>';
	htmlProduced += '</thead>';
	htmlProduced += '<tbody class="text-left for-loop-body " id="for_iteration_body">';
	htmlProduced += processWhileIteration(eventId, 1, true);
	htmlProduced += '</tbody>';
	htmlProduced += '</div>';
	return htmlProduced;
}

function requestRepeatIteration() {
	let repeatId = parseInt($('*[id^=dropSearch]').attr('id').split('-')[1]);
	if ($('*[id^=dropSearch]').val() == '') {
		$('*[id^=dropSearch]').css('background-color', 'white');
		$('*[id^=dropSearch]').css('color', 'black');
	} else {
		let requestIt = parseInt($('*[id^=dropSearch]').val());

		if (events[repeatId]['data']['iterCounter'] >= requestIt && events[repeatId]['data']['iterCounter'] >= 0) {
			processRepeatIteration(repeatId, requestIt, false);
			$('*[id^=dropSearch]').css('background-color', 'lightgreen');
			$('*[id^=dropSearch]').css('color', 'black');
		} else {
			$('*[id^=dropSearch]').css('background-color', 'tomato');
			$('*[id^=dropSearch]').css('color', 'black');
		}
	}
}
function requestNextRepeatIteration(repeatId) {
	repeatId = repeatId.split('-')[1];
	let currIteration = parseInt(document.getElementById('dropdownMenuLink').innerHTML);
	if (events[repeatId]['data']['iterCounter'] >= currIteration + 1) {
		$('*[id^=dropSearch]').css('background-color', 'white');
		$('*[id^=dropSearch]').css('color', 'black');
		$('*[id^=dropSearch]').val('');
		processRepeatIteration(repeatId, currIteration + 1, false);
	}
}
function requestPrevRepeatIteration(repeatId) {
	repeatId = repeatId.split('-')[1];
	let currIteration = parseInt(document.getElementById('dropdownMenuLink').innerHTML);
	if (currIteration - 1 > 0) {
		$('*[id^=dropSearch]').css('background-color', 'white');
		$('*[id^=dropSearch]').css('color', 'black');
		$('*[id^=dropSearch]').val('');
		processRepeatIteration(repeatId, currIteration - 1, false);
	}
}

function requestWhileIteration() {
	let whileId = parseInt($('*[id^=dropSearch]').attr('id').split('-')[1]);
	if ($('*[id^=dropSearch]').val() == '') {
		$('*[id^=dropSearch]').css('background-color', 'white');
		$('*[id^=dropSearch]').css('color', 'black');
	} else {
		let requestIt = parseInt($('*[id^=dropSearch]').val());

		if (events[whileId]['data']['iterCounter'] >= requestIt && events[whileId]['data']['iterCounter'] >= 0) {
			processWhileIteration(whileId, requestIt, false);
			$('*[id^=dropSearch]').css('background-color', 'lightgreen');
			$('*[id^=dropSearch]').css('color', 'black');
		} else {
			$('*[id^=dropSearch]').css('background-color', 'tomato');
			$('*[id^=dropSearch]').css('color', 'black');
		}
	}
}
function requestNextWhileIteration(whileId) {
	whileId = whileId.split('-')[1];
	let currIteration = parseInt(document.getElementById('dropdownMenuLink').innerHTML);
	if (events[whileId]['data']['iterCounter'] >= currIteration + 1) {
		$('*[id^=dropSearch]').css('background-color', 'white');
		$('*[id^=dropSearch]').css('color', 'black');
		$('*[id^=dropSearch]').val('');
		processWhileIteration(whileId, currIteration + 1, false);
	}
}
function requestPrevWhileIteration(whileId) {
	whileId = whileId.split('-')[1];
	let currIteration = parseInt(document.getElementById('dropdownMenuLink').innerHTML);
	if (currIteration - 1 > 0) {
		$('*[id^=dropSearch]').css('background-color', 'white');
		$('*[id^=dropSearch]').css('color', 'black');
		$('*[id^=dropSearch]').val('');
		processWhileIteration(whileId, currIteration - 1, false);
	}
}

function requestIteration() {
	let forId = parseInt($('*[id^=dropSearch]').attr('id').split('-')[1]);
	if ($('*[id^=dropSearch]').val() == '') {
		$('*[id^=dropSearch]').css('background-color', 'white');
		$('*[id^=dropSearch]').css('color', 'black');
	} else {
		let requestIt = parseInt($('*[id^=dropSearch]').val());

		if (events[forId]['data']['iterCounter'] >= requestIt && events[forId]['data']['iterCounter'] >= 0) {
			processIteration(forId, requestIt, false);
			$('*[id^=dropSearch]').css('background-color', 'lightgreen');
			$('*[id^=dropSearch]').css('color', 'black');
		} else {
			$('*[id^=dropSearch]').css('background-color', 'tomato');
			$('*[id^=dropSearch]').css('color', 'black');
		}
	}
}
function requestNextIteration(forId) {
	forId = forId.split('-')[1];
	let currIteration = parseInt(document.getElementById('dropdownMenuLink').innerHTML);

	if (events[forId]['data']['iterCounter'] >= currIteration + 1) {
		$('*[id^=dropSearch]').css('background-color', 'white');
		$('*[id^=dropSearch]').css('color', 'black');
		$('*[id^=dropSearch]').val('');
		processIteration(forId, currIteration + 1, false);
	}
}
function requestPrevIteration(forId) {
	forId = forId.split('-')[1];
	let currIteration = parseInt(document.getElementById('dropdownMenuLink').innerHTML);
	if (currIteration - 1 > 0) {
		$('*[id^=dropSearch]').css('background-color', 'white');
		$('*[id^=dropSearch]').css('color', 'black');
		$('*[id^=dropSearch]').val('');
		processIteration(forId, currIteration - 1, false);
	}
}
function resolveAuxEnvContents(env) {
	let nodeHtml = '';
	nodeHtml += '<div class="container-fluid" id="node_content">';

	let envBranchDepthInfo = envLineBDepth.get(env);
	for ([ line, content ] of auxMap.get(env)) {
		let branDepth = envBranchDepthInfo.get(line)[0];
		var numEntries = content.length - 1;
		var i;
		let auxHtml = '';
		if (content == 'Break' || content == 'Next') {
			nodeHtml += genNodeRow('', content, branDepth + 1);
		} else {
			for (i = numEntries; i >= 0; i--) {
				auxHtml += '{} {} '.format(content[i], i - 1 >= 0 ? '<-' : '');
			}
			nodeHtml += genNodeRow(line, auxHtml, branDepth);
		}
	}

	nodeHtml += '</div>';

	return nodeHtml;
}
function addEventToAuxMap(env, line, eventContent) {
	if (!auxMap.get(env).has(line)) auxMap.get(env).set(line, []);

	if (eventContent.length == 0) return;
	auxMap.get(env).get(line).push(eventContent);
}
function initAuxEnvMap(env) {
	auxMap.set(env, new Map());
	envLineBDepth.set(env, new Map());
}

let toAppendAfter = new Map();

function processIteration(forId, iterationId, toReturn) {
	if (typeof iterationId === 'string') iterationId = iterationId.split('itId-')[1];

	let iteratorState = events[forId]['data']['iterations'][iterationId]['iteratorState'];
	let iteratorId = events[forId]['data']['iteratorId'];
	let iteratorValue = getObjCurrValue(iteratorId, iteratorState, -1);
	let eventsHistory = events[forId]['data']['iterations'][iterationId]['events'];
	let htmlProduced = '';
	let i;
	htmlProduced += '<tr>';
	htmlProduced += '<td colspan="3"class="text-center">';

	htmlProduced += 'Iterator Value: {}'.format(iteratorValue[3]);
	htmlProduced += '</td>';
	htmlProduced += '</tr>';

	htmlProduced += '<tr>';
	htmlProduced += '<td>';
	auxMap = new Map();
	envLineBDepth = new Map();
	let alreadyInit = false;
	let envs = [];
	let loopHtml = '';
	let branchIncrementer = 0;

	for (i = 0; i < eventsHistory.length; i++) {
		loopHtml = '';

		eventId = eventsHistory[i];
		event = events[eventId];
		if (event['line'] == events[forId]['line'] && event['type'] == types.ASSIGN) continue;

		if (!alreadyInit) {
			alreadyInit = true;
			envs.push(event['atEnv']);
			actualEnv = envs[envs.length - 1];
			initAuxEnvMap(actualEnv);
		}

		let codeLine = code[event['line'] - 1];
		updateBranchLineDepth(actualEnv, event['line'], event['branchDepth'] + branchIncrementer);
		switch (event['type']) {
			case types.BREAK:
				addEventToAuxMap(actualEnv, event['line'], 'Break');
				break;
			case types.NEXT:
				addEventToAuxMap(actualEnv, event['line'], 'Next');
				break;
			case types.FUNC:
				let newFuncName = getCodeFlowObjNameById(event['data']['toId']);

				loopHtml += genLabelForAlreadyOpenModalWithIndent(
					eventId,
					codeLine
						.substring(
							codeLine.indexOf(newFuncName),
							codeLine.indexOf(')', codeLine.indexOf(newFuncName)) + 1
						)
						.trim(),
					0
				);

				addEventToAuxMap(actualEnv, event['line'], loopHtml);
				toAppendAfter[event['line']] = event['data']['toEnv'];
				branchIncrementer++;
				alreadyInit = false;
				break;
			case types.DATAF:
				loopHtml += genLabelForAlreadyOpenModalWithIndent(
					eventId,
					codeLine.substring(codeLine.indexOf('data.frame')).trim(),
					0
				);
				addEventToAuxMap(actualEnv, event['line'], loopHtml);
				break;
			case types.ASSIGN:
				let objId = event['data']['toObj'];
				let obj = getCommonObjNameById(objId);
				let state = event['data']['toState'];
				let origin = event['data']['origin'];
				if (origin == 'obj') {
					loopHtml += genLabelForAlreadyOpenModalWithIndent(eventId, codeLine.trim(), event['branchDepth']);
				} else {
					if (origin == 'event' && events[event['data']['fromEvent']]['type'] == types.VEC) {
						// the vector creation do not have anything special worth a separated label
						loopHtml += genLabelForAlreadyOpenModalWithIndent(eventId, codeLine.trim(), 0);
						// htmlProduced += genLabelHtml('eId-{}'.format(eventId, objId, state), codeLine.trim(), 0);
					} else {
						//other event types
						loopHtml += genLabelForAlreadyOpenModalWithIndent(
							eventId,
							codeLine
								.substring(codeLine.indexOf(obj), codeLine.indexOf('<-', codeLine.indexOf(obj)))
								.trim(),
							0
						);
					}
				}
				addEventToAuxMap(actualEnv, event['line'], loopHtml);
				break;
			case types.IF:
				let startLine = event['line'];
				let ctrJump = 0;
				let statement;
				while (event['type'] == types.IF && event['line'] == startLine) {
					//collect all if statements below
					if (event['data']['isElseIf']) {
						ctrJump++;
						statement = 'else if ( {} )'.format(event['data']['exprStr']);
					} else if (event['data']['isElse']) {
						ctrJump++;
						statement = 'else';
					} else {
						statement = 'if ( {} )'.format(event['data']['exprStr']);
					}
					loopHtml += genForMultiLabelsWithIdentAndColorAlreadyOpen(
						eventId,
						statement,
						0,
						event['data']['globalResult']
					);

					//pick next event
					event = events[++eventId];
				}
				addEventToAuxMap(actualEnv, startLine, loopHtml);
				i += ctrJump;
				break;
			case types.RET:
				if (typeof auxMap.get(event['atEnv']).get(line) === 'undefined') {
					//no content for this line, so, grab the code existing there
					loopHtml += "<ret type='button' data-toggle='modal' data-target='#exec_flow_modal' id='eId-{}' onclick='processEventClick(this.id)'> {} <- {} </ret>".format(
						eventId,
						'(return)',
						codeLine
					);
				} else {
					//already have something for this line, just append this, resolver takes care
					loopHtml += "<ret type='button' data-toggle='modal' data-target='#exec_flow_modal' id='eId-{}' onclick='processEventClick(this.id)'> {} </ret>".format(
						eventId,
						'(return)'
					);
				}
				addEventToAuxMap(actualEnv, event['line'], loopHtml);
				// htmlProduced += resolveAuxEnvContents(actualEnv);
				branchIncrementer--;
				envs.pop();
				actualEnv = envs[envs.length - 1];
				break;
			case types.ARITH:
				loopHtml += genLabelForAlreadyOpenModalWithIndent(eventId, event['data']['exprStr'].trim(), 0);
				addEventToAuxMap(event['atEnv'], event['line'], loopHtml);
				break;
			case types.IDX:
				let originIdx = event['data']['origin'];
				let toId = event['data']['toId'];
				let toState = event['data']['toState'];
				let objIdx = getCommonObjById(toId);
				if (originIdx == 'obj') {
					loopHtml += genLabelForAlreadyOpenModalWithIndent(eventId, codeLine.trim(), 0);
				} else {
					if (originIdx == 'event' && events[event['data']['fromEvent']]['type'] == types.VEC) {
						// the vector creation do not have anything special worth a separated label
						loopHtml += genLabelForAlreadyOpenModalWithIndent(eventId, codeLine.trim(), 0);
					} else {
						//other event types
						loopHtml += genLabelForAlreadyOpenModalWithIndent(
							eventId,
							codeLine
								.substring(codeLine.indexOf(objIdx), codeLine.indexOf('<-', codeLine.indexOf(objIdx)))
								.trim(),
							0
						);
					}
				}
				addEventToAuxMap(event['atEnv'], event['line'], loopHtml);
				break;
			case types.FOR: {
				let envir = event['atEnv'];
				if (codeLine.includes('{')) codeLine = codeLine.substring(0, codeLine.indexOf('{'));
				let auxHtml = genLabelForAlreadyOpenModalWithIndent(eventId, codeLine.trim(), event['branchDepth']);

				addEventToAuxMap(envir, event['line'], auxHtml);
				break;
			}
			case types.REPEAT: {
				let envir = event['atEnv'];
				if (codeLine.includes('{')) codeLine = codeLine.substring(0, codeLine.indexOf('{'));
				let auxHtml = genLabelForAlreadyOpenModalWithIndent(eventId, codeLine.trim(), event['branchDepth']);

				addEventToAuxMap(envir, event['line'], auxHtml);
				break;
			}
			case types.WHILE: {
				let envir = event['atEnv'];
				if (codeLine.includes('{')) codeLine = codeLine.substring(0, codeLine.indexOf('{'));
				let auxHtml = genLabelForAlreadyOpenModalWithIndent(eventId, codeLine.trim(), event['branchDepth']);

				addEventToAuxMap(envir, event['line'], auxHtml);
				break;
			}
			default:
				break;
		}

		if (
			event['line'] in toAppendAfter &&
			event['type'] != types.FUNC &&
			event['type'] != types.BREAK &&
			event['type'] != types.NEXT
		) {
			mergeMaps(actualEnv, toAppendAfter[event['line']]);
			toAppendAfter = new Map();
		}
	}
	htmlProduced += resolveAuxEnvContents(actualEnv);
	htmlProduced += '</td>';
	htmlProduced += '</tr>';
	if (toReturn) {
		return htmlProduced;
	} else {
		document.getElementById('dropdownMenuLink').innerHTML = iterationId;
		document.getElementById('for_iteration_body').innerHTML = htmlProduced;
		document.getElementById('for_iteration_body_header').innerHTML = 'Iteration {} information'.format(iterationId);
	}
}

function processRepeatIteration(repeatId, iterationId, toReturn) {
	if (typeof iterationId === 'string') iterationId = iterationId.split('itId-')[1];

	let eventsHistory = events[repeatId]['data']['iterations'][iterationId];
	let htmlProduced = '';
	let i;
	htmlProduced += '<tr>';
	htmlProduced += '<td>';
	auxMap = new Map();
	envLineBDepth = new Map();
	let alreadyInit = false;
	let envs = [];
	let loopHtml = '';
	let branchIncrementer = 0;

	for (i = 0; i < eventsHistory.length; i++) {
		loopHtml = '';

		eventId = eventsHistory[i];
		event = events[eventId];
		if (event['line'] == events[repeatId]['line'] && event['type'] == types.ASSIGN) continue;

		if (!alreadyInit) {
			alreadyInit = true;
			envs.push(event['atEnv']);
			actualEnv = envs[envs.length - 1];
			initAuxEnvMap(actualEnv);
		}

		let codeLine = code[event['line'] - 1];
		updateBranchLineDepth(actualEnv, event['line'], event['branchDepth'] + branchIncrementer);
		switch (event['type']) {
			case types.BREAK:
				addEventToAuxMap(actualEnv, event['line'], 'Break');
				break;
			case types.NEXT:
				addEventToAuxMap(actualEnv, event['line'], 'Next');
				break;
			case types.FUNC:
				let newFuncName = getCodeFlowObjNameById(event['data']['toId']);

				loopHtml += genLabelForAlreadyOpenModalWithIndent(
					eventId,
					codeLine
						.substring(
							codeLine.indexOf(newFuncName),
							codeLine.indexOf(')', codeLine.indexOf(newFuncName)) + 1
						)
						.trim(),
					0
				);

				addEventToAuxMap(actualEnv, event['line'], loopHtml);
				toAppendAfter[event['line']] = event['data']['toEnv'];
				branchIncrementer++;
				alreadyInit = false;
				break;
			case types.DATAF:
				loopHtml += genLabelForAlreadyOpenModalWithIndent(
					eventId,
					codeLine.substring(codeLine.indexOf('data.frame')).trim(),
					0
				);
				addEventToAuxMap(actualEnv, event['line'], loopHtml);
				break;
			case types.ASSIGN:
				let objId = event['data']['toObj'];
				let obj = getCommonObjNameById(objId);
				let state = event['data']['toState'];
				let origin = event['data']['origin'];
				if (origin == 'obj') {
					loopHtml += genLabelForAlreadyOpenModalWithIndent(eventId, codeLine.trim(), event['branchDepth']);
				} else {
					if (origin == 'event' && events[event['data']['fromEvent']]['type'] == types.VEC) {
						// the vector creation do not have anything special worth a separated label
						loopHtml += genLabelForAlreadyOpenModalWithIndent(eventId, codeLine.trim(), 0);
						// htmlProduced += genLabelHtml('eId-{}'.format(eventId, objId, state), codeLine.trim(), 0);
					} else {
						//other event types
						loopHtml += genLabelForAlreadyOpenModalWithIndent(
							eventId,
							codeLine
								.substring(codeLine.indexOf(obj), codeLine.indexOf('<-', codeLine.indexOf(obj)))
								.trim(),
							0
						);
					}
				}
				addEventToAuxMap(actualEnv, event['line'], loopHtml);
				break;
			case types.IF:
				let startLine = event['line'];
				let ctrJump = 0;
				let statement;
				while (event['type'] == types.IF && event['line'] == startLine) {
					//collect all if statements below
					if (event['data']['isElseIf']) {
						ctrJump++;
						statement = 'else if ( {} )'.format(event['data']['exprStr']);
					} else if (event['data']['isElse']) {
						ctrJump++;
						statement = 'else';
					} else {
						statement = 'if ( {} )'.format(event['data']['exprStr']);
					}
					loopHtml += genForMultiLabelsWithIdentAndColorAlreadyOpen(
						eventId,
						statement,
						0,
						event['data']['globalResult']
					);

					//pick next event
					event = events[++eventId];
				}
				addEventToAuxMap(actualEnv, startLine, loopHtml);
				i += ctrJump;
				break;
			case types.RET:
				if (typeof auxMap.get(event['atEnv']).get(line) === 'undefined') {
					//no content for this line, so, grab the code existing there
					loopHtml += "<ret type='button' data-toggle='modal' data-target='#exec_flow_modal' id='eId-{}' onclick='processEventClick(this.id)'> {} <- {} </ret>".format(
						eventId,
						'(return)',
						codeLine
					);
				} else {
					//already have something for this line, just append this, resolver takes care
					loopHtml += "<ret type='button' data-toggle='modal' data-target='#exec_flow_modal' id='eId-{}' onclick='processEventClick(this.id)'> {} </ret>".format(
						eventId,
						'(return)'
					);
				}
				addEventToAuxMap(actualEnv, event['line'], loopHtml);
				// htmlProduced += resolveAuxEnvContents(actualEnv);
				branchIncrementer--;
				envs.pop();
				actualEnv = envs[envs.length - 1];
				break;
			case types.ARITH:
				loopHtml += genLabelForAlreadyOpenModalWithIndent(eventId, event['data']['exprStr'].trim(), 0);
				addEventToAuxMap(event['atEnv'], event['line'], loopHtml);
				break;
			case types.IDX:
				let originIdx = event['data']['origin'];
				let toId = event['data']['toId'];
				let toState = event['data']['toState'];
				let objIdx = getCommonObjById(toId);
				if (originIdx == 'obj') {
					loopHtml += genLabelForAlreadyOpenModalWithIndent(eventId, codeLine.trim(), 0);
				} else {
					if (originIdx == 'event' && events[event['data']['fromEvent']]['type'] == types.VEC) {
						// the vector creation do not have anything special worth a separated label
						loopHtml += genLabelForAlreadyOpenModalWithIndent(eventId, codeLine.trim(), 0);
					} else {
						//other event types
						loopHtml += genLabelForAlreadyOpenModalWithIndent(
							eventId,
							codeLine
								.substring(codeLine.indexOf(objIdx), codeLine.indexOf('<-', codeLine.indexOf(objIdx)))
								.trim(),
							0
						);
					}
				}
				addEventToAuxMap(event['atEnv'], event['line'], loopHtml);
				break;

			case types.FOR: {
				let envir = event['atEnv'];
				if (codeLine.includes('{')) codeLine = codeLine.substring(0, codeLine.indexOf('{'));
				let auxHtml = genLabelForAlreadyOpenModalWithIndent(eventId, codeLine.trim(), event['branchDepth']);

				addEventToAuxMap(envir, event['line'], auxHtml);
				break;
			}
			case types.REPEAT: {
				let envir = event['atEnv'];
				if (codeLine.includes('{')) codeLine = codeLine.substring(0, codeLine.indexOf('{'));
				let auxHtml = genLabelForAlreadyOpenModalWithIndent(eventId, codeLine.trim(), event['branchDepth']);

				addEventToAuxMap(envir, event['line'], auxHtml);
				break;
			}
			case types.WHILE: {
				let envir = event['atEnv'];
				if (codeLine.includes('{')) codeLine = codeLine.substring(0, codeLine.indexOf('{'));
				let auxHtml = genLabelForAlreadyOpenModalWithIndent(eventId, codeLine.trim(), event['branchDepth']);

				addEventToAuxMap(envir, event['line'], auxHtml);
				break;
			}
			default:
				break;
		}

		if (
			event['line'] in toAppendAfter &&
			event['type'] != types.FUNC &&
			event['type'] != types.BREAK &&
			event['type'] != types.NEXT
		) {
			mergeMaps(actualEnv, toAppendAfter[event['line']]);
			toAppendAfter = new Map();
		}
	}
	htmlProduced += resolveAuxEnvContents(actualEnv);
	htmlProduced += '</td>';
	htmlProduced += '</tr>';
	if (toReturn) {
		return htmlProduced;
	} else {
		document.getElementById('dropdownMenuLink').innerHTML = iterationId;
		document.getElementById('for_iteration_body').innerHTML = htmlProduced;
		document.getElementById('for_iteration_body_header').innerHTML = 'Iteration {} information'.format(iterationId);
	}
}

function processWhileIteration(whileId, iterationId, toReturn) {
	if (typeof iterationId === 'string') iterationId = iterationId.split('itId-')[1];

	let eventsHistory = events[whileId]['data']['iterations'][iterationId];

	if (eventsHistory.length == 0) return '';

	let htmlProduced = '';
	let i;

	let controlStatementEvent = events[eventsHistory[0]]['data'];
	htmlProduced += '<tr>';
	htmlProduced += '<td>Loop testing: </td>';
	htmlProduced += '<td colspan="2">';

	htmlProduced += '{}'.format(
		genLabelForAlreadyOpenModalWithIndentAndColor(
			eventsHistory[0],
			controlStatementEvent['exprStr'],
			0,
			controlStatementEvent['globalResult']
		)
	);
	htmlProduced += '</td>';
	htmlProduced += '</tr>';

	htmlProduced += '<tr>';
	htmlProduced += '<td>';
	auxMap = new Map();
	envLineBDepth = new Map();
	let alreadyInit = false;
	let envs = [];
	let loopHtml = '';
	let branchIncrementer = 0;

	for (i = 1; i < eventsHistory.length; i++) {
		loopHtml = '';

		eventId = eventsHistory[i];
		event = events[eventId];
		if (event['line'] == events[whileId]['line'] && event['type'] == types.ASSIGN) continue;

		if (!alreadyInit) {
			alreadyInit = true;
			envs.push(event['atEnv']);
			actualEnv = envs[envs.length - 1];
			initAuxEnvMap(actualEnv);
		}

		let codeLine = code[event['line'] - 1];
		updateBranchLineDepth(actualEnv, event['line'], event['branchDepth'] + branchIncrementer);
		switch (event['type']) {
			case types.BREAK:
				addEventToAuxMap(actualEnv, event['line'], 'Break');
				break;
			case types.NEXT:
				addEventToAuxMap(actualEnv, event['line'], 'Next');
				break;
			case types.FUNC:
				let newFuncName = getCodeFlowObjNameById(event['data']['toId']);

				loopHtml += genLabelForAlreadyOpenModalWithIndent(
					eventId,
					codeLine
						.substring(
							codeLine.indexOf(newFuncName),
							codeLine.indexOf(')', codeLine.indexOf(newFuncName)) + 1
						)
						.trim(),
					0
				);

				addEventToAuxMap(actualEnv, event['line'], loopHtml);
				toAppendAfter[event['line']] = event['data']['toEnv'];
				branchIncrementer++;
				alreadyInit = false;
				break;
			case types.DATAF:
				loopHtml += genLabelForAlreadyOpenModalWithIndent(
					eventId,
					codeLine.substring(codeLine.indexOf('data.frame')).trim(),
					0
				);
				addEventToAuxMap(actualEnv, event['line'], loopHtml);
				break;
			case types.ASSIGN:
				let objId = event['data']['toObj'];
				let obj = getCommonObjNameById(objId);
				let state = event['data']['toState'];
				let origin = event['data']['origin'];
				if (origin == 'obj') {
					loopHtml += genLabelForAlreadyOpenModalWithIndent(eventId, codeLine.trim(), event['branchDepth']);
				} else {
					if (origin == 'event' && events[event['data']['fromEvent']]['type'] == types.VEC) {
						// the vector creation do not have anything special worth a separated label
						loopHtml += genLabelForAlreadyOpenModalWithIndent(eventId, codeLine.trim(), 0);
						// htmlProduced += genLabelHtml('eId-{}'.format(eventId, objId, state), codeLine.trim(), 0);
					} else {
						//other event types
						loopHtml += genLabelForAlreadyOpenModalWithIndent(
							eventId,
							codeLine
								.substring(codeLine.indexOf(obj), codeLine.indexOf('<-', codeLine.indexOf(obj)))
								.trim(),
							0
						);
					}
				}
				addEventToAuxMap(actualEnv, event['line'], loopHtml);
				break;
			case types.IF:
				let startLine = event['line'];
				let ctrJump = 0;
				let statement;
				while (event['type'] == types.IF && event['line'] == startLine) {
					//collect all if statements below
					if (event['data']['isElseIf']) {
						ctrJump++;
						statement = 'else if ( {} )'.format(event['data']['exprStr']);
					} else if (event['data']['isElse']) {
						ctrJump++;
						statement = 'else';
					} else {
						statement = 'if ( {} )'.format(event['data']['exprStr']);
					}
					loopHtml += genForMultiLabelsWithIdentAndColorAlreadyOpen(
						eventId,
						statement,
						0,
						event['data']['globalResult']
					);

					//pick next event
					event = events[++eventId];
				}
				addEventToAuxMap(actualEnv, startLine, loopHtml);
				i += ctrJump;
				break;
			case types.RET:
				if (typeof auxMap.get(event['atEnv']).get(line) === 'undefined') {
					//no content for this line, so, grab the code existing there
					loopHtml += "<ret type='button' data-toggle='modal' data-target='#exec_flow_modal' id='eId-{}' onclick='processEventClick(this.id)'> {} <- {} </ret>".format(
						eventId,
						'(return)',
						codeLine
					);
				} else {
					//already have something for this line, just append this, resolver takes care
					loopHtml += "<ret type='button' data-toggle='modal' data-target='#exec_flow_modal' id='eId-{}' onclick='processEventClick(this.id)'> {} </ret>".format(
						eventId,
						'(return)'
					);
				}
				addEventToAuxMap(actualEnv, event['line'], loopHtml);
				// htmlProduced += resolveAuxEnvContents(actualEnv);
				branchIncrementer--;
				envs.pop();
				actualEnv = envs[envs.length - 1];
				break;
			case types.ARITH:
				loopHtml += genLabelForAlreadyOpenModalWithIndent(eventId, event['data']['exprStr'].trim(), 0);
				addEventToAuxMap(event['atEnv'], event['line'], loopHtml);
				break;
			case types.IDX:
				let originIdx = event['data']['origin'];
				let toId = event['data']['toId'];
				let toState = event['data']['toState'];
				let objIdx = getCommonObjById(toId);
				if (originIdx == 'obj') {
					loopHtml += genLabelForAlreadyOpenModalWithIndent(eventId, codeLine.trim(), 0);
				} else {
					if (originIdx == 'event' && events[event['data']['fromEvent']]['type'] == types.VEC) {
						// the vector creation do not have anything special worth a separated label
						loopHtml += genLabelForAlreadyOpenModalWithIndent(eventId, codeLine.trim(), 0);
					} else {
						//other event types
						loopHtml += genLabelForAlreadyOpenModalWithIndent(
							eventId,
							codeLine
								.substring(codeLine.indexOf(objIdx), codeLine.indexOf('<-', codeLine.indexOf(objIdx)))
								.trim(),
							0
						);
					}
				}
				addEventToAuxMap(event['atEnv'], event['line'], loopHtml);
				break;
			case types.FOR: {
				let envir = event['atEnv'];
				if (codeLine.includes('{')) codeLine = codeLine.substring(0, codeLine.indexOf('{'));
				let auxHtml = genLabelForAlreadyOpenModalWithIndent(eventId, codeLine.trim(), event['branchDepth']);

				addEventToAuxMap(envir, event['line'], auxHtml);
				break;
			}
			case types.REPEAT: {
				let envir = event['atEnv'];
				if (codeLine.includes('{')) codeLine = codeLine.substring(0, codeLine.indexOf('{'));
				let auxHtml = genLabelForAlreadyOpenModalWithIndent(eventId, codeLine.trim(), event['branchDepth']);

				addEventToAuxMap(envir, event['line'], auxHtml);
				break;
			}
			case types.WHILE: {
				let envir = event['atEnv'];
				if (codeLine.includes('{')) codeLine = codeLine.substring(0, codeLine.indexOf('{'));
				let auxHtml = genLabelForAlreadyOpenModalWithIndent(eventId, codeLine.trim(), event['branchDepth']);

				addEventToAuxMap(envir, event['line'], auxHtml);
				break;
			}
			default:
				break;
		}

		if (
			event['line'] in toAppendAfter &&
			event['type'] != types.FUNC &&
			event['type'] != types.BREAK &&
			event['type'] != types.NEXT
		) {
			mergeMaps(actualEnv, toAppendAfter[event['line']]);
			toAppendAfter = new Map();
		}
	}
	htmlProduced += resolveAuxEnvContents(actualEnv);
	htmlProduced += '</td>';
	htmlProduced += '</tr>';
	if (toReturn) {
		return htmlProduced;
	} else {
		document.getElementById('dropdownMenuLink').innerHTML = iterationId;
		document.getElementById('for_iteration_body').innerHTML = htmlProduced;
		document.getElementById('for_iteration_body_header').innerHTML = 'Iteration {} information'.format(iterationId);
	}
}

function mergeMaps(env, withEnv) {
	let toAppendContent = auxMap.get(withEnv);
	for ([ line, content ] of toAppendContent) {
		auxMap.get(env).set(line, content);
		actualBDepth = envLineBDepth.get(withEnv).get(line)[0];
		updateBranchLineDepth(env, line, actualBDepth + 2);
	}
}

function mkIdxChangeModalInfo(event, eventId) {
	let htmlProduced = '';
	let displayNote = false;
	let targetId = event['data']['toId'];
	let targetState = event['data']['toState'];
	let targetName = getCommonObjNameById(targetId);
	let targetCurrValue = getObjCurrValue(targetId, targetState, -1);
	let targetObj = getCommonObjById(targetId);
	let repeater = 0;
	let sourceName = '';
	let foundEvent = 0;

	htmlProduced += '<div class="container-fluid">';

	htmlProduced += '<div class="row mt-2">';
	htmlProduced += '<div class="col text-left">Target Object:';
	htmlProduced += '</div>';
	htmlProduced += '<div class="col-md-auto text-left">{}'.format(targetName);
	htmlProduced += '</div>';
	htmlProduced += '</div>';
	//first section obj structure Type
	htmlProduced += '<div class="row">';
	htmlProduced += '<div class="col text-left">N. Changes:';
	htmlProduced += '</div>';
	htmlProduced += '<div class="col-md-auto text-left">{}'.format(targetCurrValue[2]);
	htmlProduced += '</div>';
	htmlProduced += '</div>';

	htmlProduced += '<div class="row">';

	if (event['data']['origin'] == 'obj') {
		htmlProduced += '<div class="col text-left">Source Object:';
		htmlProduced += '</div>';
		htmlProduced += '<div class="col-md-auto text-left">';
		if (event['data']['fromObj'] != 'HC') {
			repeater = event['data']['fromIdxs'].length;
			if (targetCurrValue[2] > repeater) {
				displayNote = true;
			}
			if (event['data']['fromObj'] == 'ABD') {
				foundEvent = findEventId(event['data']['fromId'], event['data']['fromState']);
				sourceName = getCommonObjNameById(event['data']['fromId']);
				htmlProduced += genLabelForAlreadyOpenModal(foundEvent, sourceName);
			} else {
				let firstElement = [ 'Obj', 'Not-tracked' ];

				htmlProduced += '{}'.format(mkTooltipOneLine(firstElement, event['data']['name']));
			}
		} else {
			htmlProduced += 'User-Typed';
		}
	} else {
		htmlProduced += '<div class="col text-left">Source Event:';
		htmlProduced += '</div>';
		htmlProduced += '<div class="col-md-auto text-left">';
		let sourceEvent = events[event['data']['fromEvent']];

		switch (sourceEvent['type']) {
			case types.RET:
				htmlProduced += '{}() return'.format(getCodeFlowObjNameById(sourceEvent['atFunc']));

				break;
			case types.ARITH:
				htmlProduced += genLabelForAlreadyOpenModal(event['data']['fromEvent'], 'Arithmetic');
				break;
		}
	}

	htmlProduced += '</div>';
	htmlProduced += '</div>';

	htmlProduced += '<div class="row">';
	htmlProduced += '<div class="col text-left">Final values:';
	htmlProduced += '</div>';
	htmlProduced += '<div class="col-md-auto text-left">{}'.format(mkTooltip(targetCurrValue));
	htmlProduced += '</div>';
	htmlProduced += '</div>';

	if (displayNote) {
	}

	htmlProduced += '<div class="row mt-5">';
	htmlProduced += '<div class="col-9 text-left dialog-title">Changes breakdown';
	htmlProduced += '</div>';
	htmlProduced += '</div>';

	//second section table start
	htmlProduced += '<table class="table table-sm mt-2">';
	htmlProduced += '<thead>';
	//table headers
	htmlProduced += '<tr class="dialog-text">';
	htmlProduced += '<th class ="text-center" scope="col">Target</th>';
	htmlProduced += '<th class ="text-center" scope="col">Source</th>';
	htmlProduced += '<th class ="text-center" scope="col">Value</th>';
	htmlProduced += '</tr>';
	htmlProduced += '</thead>';
	htmlProduced += '<tbody class="text-center">';
	let j, i;

	for (j = 0, i = 0; j < targetObj['modList'][targetState]['numMods']; j++, i++) {
		let currMod = targetObj['modList'][targetState]['mods'][j];
		if (i == repeater) i = 0;
		htmlProduced += '<tr>';
		htmlProduced += '<td>{}[{}]</td>'.format(targetName, currMod['index'] + 1);
		htmlProduced += '<td>';
		if (event['data']['origin'] == 'obj') {
			switch (event['data']['fromObj']) {
				case 'HC':
					htmlProduced += 'U-T';
					//hardcoded value
					break;
				case 'R':
					//r object
					htmlProduced += '{}[{}]'.format(event['data']['name'], i + 1);
					break;
				default:
					htmlProduced += '{}[{}]'.format(sourceName, i + 1);
					//abd object
					break;
			}
		} else {
			let sourceEvent = events[event['data']['fromEvent']];

			switch (sourceEvent['type']) {
				case types.RET:
					htmlProduced += '{}() return'.format(getCodeFlowObjNameById(sourceEvent['atFunc']));

					break;
				case types.ARITH:
					htmlProduced += sourceEvent['data']['exprStr'];
					break;
			}
		}
		htmlProduced += '</td>';
		htmlProduced += '<td>{}</td>'.format(currMod['newValue']);

		htmlProduced += '</tr>';
	}

	htmlProduced += '</tbody>';
	htmlProduced += '</table>';

	htmlProduced += '</div>';
	return htmlProduced;
}

//ONE DIM
function genForOneDim() {
	let htmlProduced = '';
	let nCols = valuesToBigData[2];

	if (nCols > 10) nCols = 10;

	htmlProduced += '<div class="container-fluid">';

	htmlProduced += '<div class="row mt-2 dialog-text">';
	htmlProduced += '<div class="col text-left">Elements count: </div>';
	htmlProduced += '<div class="col-md-auto text-left">{}</div>'.format(valuesToBigData[2]);
	htmlProduced += '</div>';

	htmlProduced += '<div class="row mt-5">';
	htmlProduced += '<div class="col-9 text-left dialog-title">Values';
	htmlProduced += '</div>';
	htmlProduced += '</div>';

	//navigation

	htmlProduced += '<div class="row mt-3">';
	htmlProduced += '<div class="col text-left dialog-text">Display <input id="big_n_cols_OD" type="text" style="width:50px;" value="{}"/> elements'.format(
		nCols
	);

	htmlProduced +=
		'<i class="fa fa-refresh" style="font-size:22px;margin-left:10px;c	olor:var(--title-color);cursor: pointer;" onclick="requestFrameNColsTableUpdate_BigDataOneDim()"></i>';
	htmlProduced += '</div>';
	htmlProduced += '</div>';

	htmlProduced += '<div class="row mt-3 dialog-text">';
	htmlProduced += '<div class="col text-center">Prev. page';

	htmlProduced += '</div>';
	htmlProduced += '<div class="col text-center">Start index';

	htmlProduced += '</div>';
	htmlProduced += '<div class="col text-center">Next page';
	htmlProduced += '</div>';
	htmlProduced += '</div>';

	htmlProduced += '<div class="row mt-1 dialog-text">';
	htmlProduced += '<div class="col text-center">';
	htmlProduced +=
		"<i class='fa fa-arrow-left' aria-hidden='true' style='font-size:22px;margin-right:10px;color:var(--title-color);cursor: pointer;' onclick='requestPrevPageChange_BigDataOneDim()'></i>";

	htmlProduced += '</div>';
	htmlProduced += '<div class="col text-center">';
	htmlProduced += '<input id="start_index_big_OD" type="text" style="width:50px;" value="1"/>';
	htmlProduced += '</div>';
	htmlProduced += '<div class="col text-center">';
	htmlProduced +=
		"<i class='fa fa-arrow-right' aria-hidden='true' style='font-size:22px;margin-right:10px;color:var(--title-color);cursor: pointer;' onclick='requestNextPageChange_BigDataOneDim()'></i>";

	htmlProduced += '</div>';
	htmlProduced += '</div>';

	htmlProduced += '<div class="row mt-2 dialog-text">';
	htmlProduced += '<div class="col text-center">';
	htmlProduced += '<table class="table table-sm mt-2 dialog-text">';
	htmlProduced += '<tbody class="text-center" id="frame_body">';
	htmlProduced += updateBigDataValuesTable_OneDim(0, nCols, true);
	htmlProduced += '</tbody>';
	htmlProduced += '</table>';
	htmlProduced += '</div>';
	htmlProduced += '</div>';

	htmlProduced += '</div>';
	return htmlProduced;
}

function requestFrameNColsTableUpdate_BigDataOneDim() {
	let val = parseInt($('*[id^=big_n_cols_OD]').val());
	let startIdx = parseInt($('*[id^=start_index_big_OD]').val()) - 1;
	if (startIdx < 0 || startIdx >= valuesToBigData[2]) {
		$('*[id^=start_index_big_OD]').val(1);
		startIdx = 0;
	}
	if (val > 0) updateBigDataValuesTable_OneDim(startIdx, val, false);
}

function requestStartIdxUpdate_BigDataOneDim() {
	let val = parseInt($('*[id^=big_n_cols_OD]').val());
	let startIdx = parseInt($('*[id^=start_index_big_OD]').val()) - 1;
	if (startIdx < 0 || startIdx >= valuesToBigData[2]) {
		$('*[id^=start_index_big_OD]').val(1);
		startIdx = 0;
	}
	if (val > 0) updateBigDataValuesTable_OneDim(startIdx, val, false);
}

function requestPrevPageChange_BigDataOneDim() {
	let val = parseInt($('*[id^=big_n_cols_OD]').val());
	let startIdx = parseInt($('*[id^=start_index_big_OD]').val()) - 1;
	if (startIdx < 0) {
		$('*[id^=start_index_big_OD]').val(1);
		startIdx = 0;
	}
	let need = startIdx - val;

	if (need < 0) need = 0;
	$('*[id^=start_index_big_OD]').val(need + 1);
	if (val > 0) updateBigDataValuesTable_OneDim(need, val, false);
}

function requestNextPageChange_BigDataOneDim() {
	let val = parseInt($('*[id^=big_n_cols_OD]').val());
	let startIdx = parseInt($('*[id^=start_index_big_OD]').val()) - 1;
	if (startIdx < 0 || startIdx >= valuesToBigData[2]) {
		$('*[id^=start_index_big_OD]').val(1);
		startIdx = 0;
	}
	let maxCols = valuesToBigData[2];
	let need = startIdx + val;
	if (need >= maxCols) return;
	$('*[id^=start_index_big_OD]').val(need + 1);
	if (val > 0) updateBigDataValuesTable_OneDim(need, val, false);
}

function updateBigDataValuesTable_OneDim(startingIdx, nCols, toReturn) {
	let htmlProduced = '';
	let idxCol = '';
	let valCol = '';
	let i;
	if (isNaN(startingIdx)) {
		startingIdx = 0;
		$('*[id^=start_index_big_OD]').val(1);
	}
	if (isNaN(nCols)) {
		nCols = 5;
		$('*[id^=big_n_cols_OD]').val(5);
	}

	if (nCols > valuesToBigData[2]) {
		nCols = valuesToBigData[2];
	}

	if (startingIdx + nCols > valuesToBigData[2]) {
		nCols = valuesToBigData[2];
	} else {
		nCols += startingIdx;
	}

	idxCol += '<tr>';
	idxCol += '<td><b>Index</b></td>';
	valCol += '<tr>';
	valCol += '<td><b>Value</b></td>';

	for (i = startingIdx; i < nCols; i++) {
		idxCol += '<td> {}</td>'.format(i + 1);
		valCol += '<td> {}</td>'.format(valuesToBigData[3][i]);
	}
	valCol += '</tr>';
	idxCol += '</tr>';
	htmlProduced += idxCol + valCol;
	if (toReturn) return htmlProduced;
	else document.getElementById('frame_body').innerHTML = htmlProduced;
}

//MULTI DIM
function genForMultiDim() {
	let htmlProduced = '';
	valuesToBigData[2] = valuesToBigData[2].split('by');
	let nRows = valuesToBigData[2][0];
	let nCols = valuesToBigData[2][1];

	if (nCols > 10) nCols = 10;
	if (nRows > 10) nRows = 10;

	htmlProduced += '<div class="container-fluid">';

	htmlProduced += '<div class="row mt-2 dialog-text">';
	htmlProduced += '<div class="col text-left">Rows count: </div>';
	htmlProduced += '<div class="col-md-auto text-left">{}</div>'.format(valuesToBigData[2][0]);
	htmlProduced += '</div>';
	htmlProduced += '<div class="row mt-2 dialog-text">';
	htmlProduced += '<div class="col text-left">Columns count: </div>';
	htmlProduced += '<div class="col-md-auto text-left">{}</div>'.format(valuesToBigData[2][1]);
	htmlProduced += '</div>';

	htmlProduced += '<div class="row">';
	htmlProduced +=
		'<div class="col text-left mt-2" style="color:red; font-size:11pt;">Note: To input use <row,col> notation (without signs)';
	htmlProduced += '</div>';
	htmlProduced += '</div>';
	htmlProduced += '<div class="row mt-5">';
	htmlProduced += '<div class="col-9 text-left dialog-title">Values';
	htmlProduced += '</div>';
	htmlProduced += '</div>';

	//navigation

	htmlProduced += '<div class="row mt-3">';
	htmlProduced += '<div class="col text-left dialog-text">Display window:<input id="big_n_cols_OD" type="text" style="width:50px;margin-left:10px;" value="{}"/>'.format(
		'{},{}'.format(nRows, nCols)
	);

	htmlProduced +=
		'<i class="fa fa-refresh" style="font-size:22px;margin-left:10px;c	olor:var(--title-color);cursor: pointer;" onclick="requestFrameNColsTableUpdate_BigDataOneDim()"></i>';

	htmlProduced += '</div>';
	htmlProduced += '</div>';

	htmlProduced += '<div class="row mt-3 dialog-text">';
	htmlProduced += '<div class="col text-center">Prev. page';

	htmlProduced += '</div>';
	htmlProduced += '<div class="col text-center">Start cell';

	htmlProduced += '</div>';
	htmlProduced += '<div class="col text-center">Next page';
	htmlProduced += '</div>';
	htmlProduced += '</div>';

	htmlProduced += '<div class="row mt-1 dialog-text">';
	htmlProduced += '<div class="col text-center">';
	htmlProduced +=
		"<i class='fa fa-arrow-left' aria-hidden='true' style='font-size:22px;margin-right:10px;color:var(--title-color);cursor: pointer;' onclick='requestPrevPageChange_BigDataOneDim()'></i>";
	htmlProduced +=
		"<i class='fa fa-arrow-up' aria-hidden='true' style='font-size:22px;margin-right:10px;color:var(--title-color);cursor: pointer;' onclick='requestPrevPageChange_BigDataOneDim()'></i>";

	htmlProduced += '</div>';
	htmlProduced += '<div class="col text-center" >';
	htmlProduced += '<input id="start_index_big_OD" type="text" style="width:50px;" value="1,1"/>';
	htmlProduced += '</div>';

	htmlProduced += '<div class="col text-center">';
	htmlProduced +=
		"<i class='fa fa-arrow-right' aria-hidden='true' style='font-size:22px;margin-right:10px;color:var(--title-color);cursor: pointer;' onclick='requestNextPageChange_BigDataOneDim()'></i>";
	htmlProduced +=
		"<i class='fa fa-arrow-down' aria-hidden='true' style='font-size:22px;margin-right:10px;color:var(--title-color);cursor: pointer;' onclick='requestPrevPageChange_BigDataOneDim()'></i>";
	htmlProduced += '</div>';
	htmlProduced += '</div>';

	htmlProduced += '<div class="row mt-2 dialog-text">';
	htmlProduced += '<div class="col text-center">';
	htmlProduced += '<table class="table table-sm mt-2 dialog-text">';
	htmlProduced += '<tbody class="text-center" id="frame_body">';
	htmlProduced += updateBigDataValuesTable_MultiDim(0, nRows, nCols, true);
	htmlProduced += '</tbody>';
	htmlProduced += '</table>';
	htmlProduced += '</div>';
	htmlProduced += '</div>';

	htmlProduced += '</div>';
	return htmlProduced;
}

function genModalForBigData() {
	let htmlProduced = '';
	if (typeof valuesToBigData[2] == 'string') htmlProduced = genForMultiDim();
	else htmlProduced = genForOneDim();
	return htmlProduced;
}
function updateBigDataValuesTable_MultiDim(startingIdx, nRows, nCols, toReturn) {
	let htmlProduced = '';
	let i;

	if (isNaN(startingIdx)) {
		startingIdx = 0;
		$('*[id^=start_index_big]').val(1);
	}
	if (isNaN(nCols)) {
		nCols = 5;
		$('*[id^=big_n_cols]').val(5);
	}

	if (nCols > valuesToBigData[2][1]) {
		nCols = valuesToBigData[2][1];
	}
	if (nRows > valuesToBigData[2][0]) {
		nRows = valuesToBigData[2][0];
	}

	// if (startingIdx + nCols > valuesToBigData[2]) {
	// 	nCols = valuesToBigData[2];
	// } else {
	// 	nCols += startingIdx;
	// }

	console.log(valuesToBigData);
	let j;
	htmlProduced += '<tr>';
	htmlProduced += '<td><b>Indexes</b></td>';
	for (i = startingIdx; i < nCols; i++) {
		htmlProduced += '<td><b>{}</b></td>'.format(i + 1);
	}
	htmlProduced += '</tr>';
	for (j = 0; j < nRows; j++) {
		htmlProduced += '<tr>';
		htmlProduced += '<td><b>{}</b></td>'.format(j + 1);
		for (i = startingIdx; i < nCols; i++) {
			htmlProduced += '<td>{}</td>'.format(valuesToBigData[3][i][j]);
		}
		htmlProduced += '</tr>';
	}

	if (toReturn) return htmlProduced;
	else document.getElementById('frame_body').innerHTML = htmlProduced;
}

function mkDataFrameModal(event, eventId) {
	let htmlProduced = '';
	let nCols = event['data']['numCols'];

	htmlProduced += '<div class="container-fluid">';

	htmlProduced += '<div class="row mt-2 dialog-text">';
	htmlProduced += '<div class="col text-left">Columns count: </div>';
	htmlProduced += '<div class="col-md-auto text-left">{}</div>'.format(nCols);
	htmlProduced += '</div>';
	let i;
	//second section table start
	htmlProduced += '<div class="row mt-5">';
	htmlProduced += '<div class="col-9 text-left dialog-title">Sources';
	htmlProduced += '</div>';
	htmlProduced += '</div>';
	if (nCols > 5) {
		nCols = 5;
	}
	htmlProduced += '<div class="row mt-3">';
	htmlProduced += '<div class="col text-left dialog-text">Display <input id="frame_n_cols-{}" type="text" style="width:50px;" value="{}"/> columns'.format(
		eventId,
		nCols
	);

	htmlProduced +=
		'<i class="fa fa-refresh" style="font-size:22px;margin-left:10px;color:var(--title-color);cursor: pointer;" onclick="requestFrameNColsTableUpdate()"></i>';
	htmlProduced += '</div>';
	htmlProduced += '</div>';

	htmlProduced += '<div class="row mt-3 dialog-text">';
	htmlProduced += '<div class="col text-center">Prev. page';

	htmlProduced += '</div>';
	htmlProduced += '<div class="col text-center">Display from';

	htmlProduced += '</div>';
	htmlProduced += '<div class="col text-center">Next page';
	htmlProduced += '</div>';
	htmlProduced += '</div>';

	htmlProduced += '<div class="row mt-1 dialog-text">';
	htmlProduced += '<div class="col text-center">';
	htmlProduced +=
		"<i class='fa fa-arrow-left' aria-hidden='true' style='font-size:22px;margin-right:10px;color:var(--title-color);cursor: pointer;' onclick='requestPrevPageChange()'></i>";

	htmlProduced += '</div>';
	htmlProduced += '<div class="col text-center">';
	htmlProduced += '<input id="start_idx" type="text" style="width:50px;" value="1"/>';
	htmlProduced += '</div>';
	htmlProduced += '<div class="col text-center">';
	htmlProduced +=
		"<i class='fa fa-arrow-right' aria-hidden='true' style='font-size:22px;margin-right:10px;color:var(--title-color);cursor: pointer;' onclick='requestNextPageChange()'></i>";

	htmlProduced += '</div>';
	htmlProduced += '</div>';

	htmlProduced += '<table class="table table-sm mt-2 dialog-text">';
	htmlProduced += '<thead>';
	//table headers
	htmlProduced += '<tr class="dialog-text">';
	htmlProduced += '<th class ="text-center" scope="col">Col.</th>';
	htmlProduced += '<th class ="text-center" scope="col">Col. Name</th>';
	htmlProduced += '<th class ="text-center" scope="col">Object</th>';
	htmlProduced += '<th class ="text-center" scope="col">Used indexes</th>';
	htmlProduced += '<th class ="text-center" scope="col">Values</th>';
	htmlProduced += '</tr>';
	htmlProduced += '</thead>';
	htmlProduced += '<tbody class="text-center" id="frame_body">';
	htmlProduced += updateDataFrameDisplayedCols(0, nCols, event, true);
	htmlProduced += '</tbody>';
	htmlProduced += '</table>';

	htmlProduced += '</div>';
	return htmlProduced;
}
function requestFrameNColsTableUpdate() {
	let event = events[parseInt($('*[id^=frame_n_cols]').attr('id').split('-')[1])];
	let val = parseInt($('*[id^=frame_n_cols]').val());
	let startIdx = parseInt($('*[id^=start_idx]').val()) - 1;
	if (startIdx < 0 || startIdx >= event['data']['numCols']) {
		$('*[id^=start_idx]').val(1);
		startIdx = 0;
	}
	if (val > 0) updateDataFrameDisplayedCols(startIdx, val, event, false);
}

function requestStartIdxUpdate() {
	let event = events[parseInt($('*[id^=frame_n_cols]').attr('id').split('-')[1])];
	let val = parseInt($('*[id^=frame_n_cols]').val());
	let startIdx = parseInt($('*[id^=start_idx]').val()) - 1;
	if (startIdx < 0 || startIdx >= event['data']['numCols']) {
		$('*[id^=start_idx]').val(1);
		startIdx = 0;
	}
	if (val > 0) updateDataFrameDisplayedCols(startIdx, val, event, false);
}

function requestPrevPageChange() {
	let event = events[parseInt($('*[id^=frame_n_cols]').attr('id').split('-')[1])];
	let val = parseInt($('*[id^=frame_n_cols]').val());
	let startIdx = parseInt($('*[id^=start_idx]').val()) - 1;
	if (startIdx < 0) {
		$('*[id^=start_idx]').val(1);
		startIdx = 0;
	}
	let need = startIdx - val;

	if (need < 0) need = 0;
	$('*[id^=start_idx]').val(need + 1);
	if (val > 0) updateDataFrameDisplayedCols(need, val, event, false);
}

function requestNextPageChange() {
	let event = events[parseInt($('*[id^=frame_n_cols]').attr('id').split('-')[1])];
	let val = parseInt($('*[id^=frame_n_cols]').val());
	let startIdx = parseInt($('*[id^=start_idx]').val()) - 1;
	if (startIdx < 0 || startIdx >= event['data']['numCols']) {
		$('*[id^=start_idx]').val(1);
		startIdx = 0;
	}
	let maxCols = event['data']['numCols'];
	let need = startIdx + val;
	if (need >= maxCols) return;
	$('*[id^=start_idx]').val(need + 1);
	if (val > 0) updateDataFrameDisplayedCols(need, val, event, false);
}

function updateDataFrameDisplayedCols(startingIdx, nCols, event, toReturn) {
	let htmlProduced = '';
	if (isNaN(startingIdx)) {
		startingIdx = 0;
		$('*[id^=start_idx]').val(1);
	}
	if (isNaN(nCols)) {
		nCols = 5;
		$('*[id^=frame_n_cols]').val(5);
	}

	let sources = event['data']['srcs'];
	if (nCols > event['data']['numCols']) {
		nCols = event['data']['numCols'];
	}

	if (startingIdx + nCols > event['data']['numCols']) {
		nCols = event['data']['numCols'];
	} else {
		nCols += startingIdx;
	}
	var objVal;
	for (i = startingIdx; i < nCols; i++) {
		let source = sources[i];
		let objId = source['objId'];

		let values = [];
		let objName;
		let idxLabel;
		let valuesLbl = '';

		if (objId > 0) {
			objName = getCommonObjNameById(objId);
			let lastEId = findEventId(objId, source['objState']);
			objName = genLabelForAlreadyOpenModal(lastEId, objName);
			objVal = getObjCurrValue(objId, source['objState'], -1);
		} else {
			if (objId == -2) {
				objName = mkTooltipOneLine([ 'Object: ', 'Non-Tracked' ], source['name']);
				objVal = [ 'Vector', '', source['values'].length, source['values'] ];
			} else {
				objName = 'U-T';
				objVal = [ 'Vector', '', source['values'].length, source['values'] ];
			}
		}

		let idxsLen = source['idxs'].length;
		if (idxsLen == 0) {
			idxLabel = 'All';
		} else {
			if (idxsLen > 5) {
				idxLabel = mkTooltip([ 'Vector', '', idxsLen, source['idxs'] ]);
			} else {
				idxLabel = '[{}]'.format(String(source['idxs']));
			}
		}

		htmlProduced += '<tr>';
		htmlProduced += '<td>';
		htmlProduced += '{}'.format(i + 1);
		htmlProduced += '</td>';

		htmlProduced += '<td>';
		htmlProduced += '{}'.format(source['colName']);
		htmlProduced += '</td>';

		htmlProduced += '<td>';
		htmlProduced += '{}'.format(objName);
		htmlProduced += '</td>';

		htmlProduced += '<td>';
		htmlProduced += '{}'.format(idxLabel);
		htmlProduced += '</td>';

		htmlProduced += '<td>';
		htmlProduced += '{}'.format(structToStr(objVal));
		htmlProduced += '</td>';
		htmlProduced += '</tr>';
	}

	if (toReturn) return htmlProduced;
	else document.getElementById('frame_body').innerHTML = htmlProduced;
}

function produceModalContent(eventId) {
	let content = {
		title: '',
		body: ''
	};
	if (eventId.indexOf('bd') > -1) {
		content.title = 'Values Visualizer';
		content.body = genModalForBigData();
		return content;
	}
	eventId = eventId.split('-')[1];
	let event = events[eventId];
	switch (event['type']) {
		case types.DATAF:
			content.title = 'Data Frame creation';
			content.body = mkDataFrameModal(event, eventId);
			break;
		case types.ASSIGN:
			content.title = 'Object analysis';
			content.body = mkObjModalTopInfo(event);
			content.body += mkObjModalBotInfo(event, eventId);
			break;
		case types.FUNC:
			content.title = 'Function call analysis';
			content.body = mkFuncModalInfo(event, eventId);
			break;

		case types.IF:
			content.title = 'Branch analysis';
			content.body = mkIfModalInfo(event, eventId);
			break;

		case types.ARITH:
			content.title = 'Arithmetic analysis';
			content.body = mkArithModalInfo(event, eventId);
			break;
		case types.IDX:
			content.title = 'Index change analysis';
			content.body = mkIdxChangeModalInfo(event, eventId);
			break;
		case types.RET:
			content.title = 'Return analysis';
			content.body = 'nothing to show';
			break;
		case types.FOR:
			content.title = 'For-Loop analysis';
			content.body = mkForLoopModalInfo(event, eventId);
			break;
		case types.REPEAT:
			content.title = 'Repeat-Loop analysis';
			content.body = mkRepeatLoopModalInfo(event, eventId);
			break;
		case types.WHILE:
			content.title = 'While-Loop analysis';
			content.body = mkWhileLoopModalInfo(event, eventId);
			break;
		default:
			content.title = 'Ups...';
			content.body = 'Problem rendering event information';
	}

	return content;
}

let visualStack = [];
function goBackVisual() {
	let producedContent = visualStack[visualStack.length - 1];
	document.getElementById('exec_flow_modal_title').innerHTML = producedContent.title;
	document.getElementById('exec_flow_modal_body').innerHTML = producedContent.body;
	visualStack.pop();
}
function getArrowBack() {
	return "<i class='fa fa-arrow-left' aria-hidden='true' style='font-size:22px;margin-right:10px;color:var(--title-color);cursor: pointer;' onclick='goBackVisual()'></i>";
}
let modalIsOpen = false;
function processEventClick(eventId) {
	let producedContent = produceModalContent(eventId);
	if (modalIsOpen) {
		let currModalContent = {
			title: '',
			body: ''
		};
		currModalContent.title = document.getElementById('exec_flow_modal_title').innerHTML;
		currModalContent.body = document.getElementById('exec_flow_modal_body').innerHTML;
		visualStack.push(currModalContent);
		producedContent.title = '{}{}'.format(getArrowBack(), producedContent.title);
	}
	modalIsOpen = true;

	// if (visualStack.length > 0) {
	// 	producedContent.title = '{}{}'.format(getArrowBack(), producedContent.title);
	// }

	document.getElementById('exec_flow_modal_title').innerHTML = producedContent.title;
	document.getElementById('exec_flow_modal_body').innerHTML = producedContent.body;
	if (search) doSearch();
	$('[data-toggle="tooltip"]').tooltip();
	$('[data-toggle="tooltip"]').tooltip({ boundary: 'window' });
	/*$('[data-toggle="tooltip"]').tooltip({ container: 'body' }); */
	//$('[data-toggle="popover"]').popover();

	/* $('.popover-dismiss').popover({
		trigger: 'focus'
	}); */
	$('#exec_flow_modal').on('hidden.bs.modal', function() {
		visualStack = [];
		modalIsOpen = false;
	});
	$('*[id^=dropSearch]').on('keyup paste', function(event) {
		if (document.getElementById('exec_flow_modal_title').innerHTML.includes('Repeat')) {
			requestRepeatIteration();
		} else if (document.getElementById('exec_flow_modal_title').innerHTML.includes('For')) {
			requestIteration();
		}

		if (event.keyCode == 13) {
			$('#dropdownMenuLink').click();
		}
	});
	$('*[id^=start_idx]').on('keyup', function(event) {
		if (event.keyCode == 13) {
			requestStartIdxUpdate();
		}
	});

	$('*[id^=frame_n_cols]').on('keyup', function(event) {
		if (event.keyCode == 13) {
			requestFrameNColsTableUpdate();
		}
	});

	$('*[id^=start_index_big_OD]').on('keyup', function(event) {
		if (event.keyCode == 13) {
			requestStartIdxUpdate_BigDataOneDim();
		}
	});

	$('*[id^=big_n_cols_OD]').on('keyup', function(event) {
		if (event.keyCode == 13) {
			requestFrameNColsTableUpdate_BigDataOneDim();
		}
	});

	$('#dropdownMenuLink').click(function() {
		$('*[id^=dropSearch]').focus();
	});
}
/*








	


	d3 functions



*/
var graph = {
	nodes: [],
	links: []
};
function generateSVGgraph() {
	nodeSizeScale = d3.scaleLinear().domain(d3.extent(graph.nodes, (d) => d.id)).range([ 30, 30 ]);

	simulation = d3
		.forceSimulation()
		.force('link', d3.forceLink().id((d) => d.source))
		.force('charge', d3.forceManyBody())
		.force('center', d3.forceCenter(100, 100))
		.nodes(graph.nodes);

	var link_force = d3.forceLink(graph.links).id(function(d) {
		return d.name;
	});

	charge_force = d3.forceManyBody().strength(-55000);

	center_force = d3.forceCenter(width / 2, height / 2);

	simulation.force('charge_force', charge_force).force('center_force', center_force).force('link', link_force);

	//add tick instructions:
	simulation.on('tick', tickActions);

	svg = d3.select('#exec-flow-graph').attr('width', width).attr('height', height);

	svg
		.append('defs')
		.selectAll('marker')
		.data([ 'dominating' ])
		.enter()
		.append('marker')
		.attr('markerUnits', 'userSpaceOnUse')
		.attr('id', function(d) {
			return d;
		})
		.attr('viewBox', '0 -5 10 10')
		.attr('refX', 0)
		.attr('refY', 0)
		.attr('markerWidth', 12)
		.attr('markerHeight', 12)
		.attr('orient', 'auto-start-reverse')
		.append('path')
		.attr('d', 'M 0, -5 L 10 ,0 L 0,5')
		.attr('fill', '#d9b310');

	//add encompassing group for the zoom
	g = svg.append('g').attr('class', 'everything').attr('width', width).attr('height', height);

	svg.selectAll('.nodes').exit().remove();

	links = g
		.selectAll('.link')
		.data(graph.links)
		.enter()
		.append('path')
		.style('stroke', 'white')
		.attr('fill', 'transparent')
		.attr('stroke-opacity', 0.8)
		.attr('stroke-width', 2)
		.attr('marker-end', 'url(#dominating)');

	nodes = g
		.append('g')
		.attr('class', 'nodes')
		.selectAll('foreignObject')
		.data(graph.nodes)
		.enter()
		.append('foreignObject')
		.attr('id', (d) => d.id)
		.on('mouseover', mouseOver(0.1))
		.on('mouseout', mouseOut);

	drag_handler = d3.drag().on('start', drag_start).on('drag', drag_drag);
	drag_handler(nodes);

	graph.nodes.forEach((node) => {
		document.getElementById(node.id).innerHTML = node.html;
	});

	var zoom_handler = d3.zoom().on('zoom', zoom_actions);

	zoom_handler(svg);

	graph.links.forEach(function(d) {
		linkedByIndex[d.source.index + ',' + d.target.index] = 1;
	});
	g.transition().duration(300).call(zoom_handler.transform, d3.zoomIdentity);
}

function zoom_actions() {
	g.attr('transform', d3.event.transform);
}

function drag_start(d) {
	if (!d3.event.active) simulation.alphaTarget(0.3).restart();
	//d3.select(this).classed('fixed', (d.fixed = true));
	d.fx = d.x;
	d.fy = d.y;
}

//make sure you can't drag the circle outside the box
function drag_drag(d) {
	d.fx = d3.event.x;
	d.fy = d3.event.y;
}

function tickActions() {
	//update circle positions each tick of the simulation

	nodes
		.attr('x', function(d) {
			return d.x;
		})
		.attr('y', function(d) {
			return d.y;
		});

	// update link positions
	links.attr('d', positionLink1);
	links
		.filter(function(d) {
			return JSON.stringify(d.target) !== JSON.stringify(d.source);
		})
		.attr('d', positionLink2);
}

function positionLink2(d) {
	return positionLink1(d);
}

function positionLink1(d) {
	var dr = 0;
	let min = Number.MAX_SAFE_INTEGER;
	let best = {
		s: { x: 0, y: 0 },
		d: { x: 0, y: 0 }
	};
	let width = document.getElementById('env-{}'.format(d.source.name)).offsetWidth;
	let sourceHeight = document.getElementById('env-{}'.format(d.source.name)).offsetHeight;
	let targetHeight = document.getElementById('env-{}'.format(d.target.name)).offsetHeight;
	let sourceDims = [ [ width / 2, 0 ], [ width, sourceHeight / 2 ], [ width / 2, sourceHeight ], [ 0, width / 2 ] ];
	let targetDims = [ [ width / 2, 0 ], [ width, targetHeight / 2 ], [ width / 2, targetHeight ], [ 0, width / 2 ] ];

	sourceDims.forEach((s) =>
		targetDims.forEach((t) => {
			let dist = Math.hypot(d.target.x + t[0] - (d.source.x + s[0]), d.target.y + t[1] - (d.source.y + s[1]));
			if (dist < min) {
				min = dist;
				best = {
					s: { x: d.source.x + s[0], y: d.source.y + s[1] },
					d: { x: d.target.x + t[0], y: d.target.y + t[1] }
				};
			}
		})
	);

	return (
		'M' +
		best.s.x +
		',' +
		best.s.y +
		'A' +
		dr +
		',' +
		dr +
		' 0 0,1 ' +
		(best.d.x == d.target.x ? best.d.x - 10 : best.d.x + 5) +
		',' +
		(best.d.y == d.target.y ? best.d.y - 10 : best.d.y + 10)
	);
}

// check the dictionary to see if nodes are linked
function isConnected(a, b) {
	return linkedByIndex[a.index + ',' + b.index] || linkedByIndex[b.index + ',' + a.index] || a.index == b.index;
}

function mouseOver(opacity) {
	return function(d) {
		// check all other nodes to see if they're connected
		// to this one. if so, keep the opacity at 1, otherwise
		// fade
		nodes.style('stroke-opacity', function(o) {
			thisOpacity = isConnected(d, o) ? 1 : opacity;
			return thisOpacity;
		});
		nodes.style('fill-opacity', function(o) {
			thisOpacity = isConnected(d, o) ? 1 : opacity;
			return thisOpacity;
		});

		// also style link accordingly
		links.style('stroke-opacity', function(o) {
			return o.source === d || o.target === d ? 1 : opacity;
		});
		links.style('stroke', function(o) {
			return o.source === d || o.target === d ? '#f73b3b' : 'white';
		});
	};
}

function mouseOut() {
	nodes.style('stroke-opacity', 1);
	nodes.style('fill-opacity', 1);

	links.style('stroke-opacity', 1);
	links.style('stroke', 'white');
}
