var stack = [];
var stackSize = 0;
var currentEnv = '';

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
			if (events[i]['type'] != types.IF) {
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

	//line of code
	htmlProduced += '<div class="col col-lg-2 col1">';
	htmlProduced += '<label class="float-right object-rhs-special">' + line + '</label>';
	htmlProduced += '</div>';

	//code
	htmlProduced += '<div class="col col-md-auto col3" style="margin-left:{}px;">'.format(branchDept * 10);
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
	let valuesStr = structToStr(objCurrentValues);
	return "<a href='#' type='button' data-placement='right' data-toggle='tooltip' data-html='true' title='Size: {}</br>{}'><u>values!</u></a>".format(
		objCurrentValues[3].length,
		valuesStr
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

				prevChange = '<a href="#" id="eId-{}" onclick="processEventClick(this.id)"><u>line {}</u></a>'.format(
					foundEvent,
					events[foundEvent]['line']
				);
			}

			htmlProduced += '<tr>';
			htmlProduced += '<td>{}</td>'.format(fromObj);
			htmlProduced += '<td>{}</td>'.format(toObj);
			htmlProduced += '<td>{}</td>'.format(
				objCurrentValues[2] > 5 ? mkTooltip(objCurrentValues) : structToStr(objCurrentValues)
			);
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
			case types.VEC:
				sourceEvent = 'Vector Creation';
				break;
			case types.RET:
				sourceEvent = '{}() return'.format(
					getCodeFlowObjNameById(events[event['data']['fromEvent']]['atFunc'])
				);
				break;
			case types.ARITH:
				sourceEvent = 'Arithmetic';
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

	if (objCurrentValues[2] > 5) {
		//create popover to display information
		htmlProduced += mkTooltip(objCurrentValues);
	} else {
		htmlProduced += structToStr(objCurrentValues);
	}
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

					if (objStateValues[2] > 4) {
						//create popover to display information
						strToAppend += mkTooltip(objStateValues);
					} else {
						strToAppend += structToStr(objStateValues);
					}
					htmlProduced += '<td class ="text-center">{}</td>'.format(strToAppend);
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

			let html = '{} <- <a href="#" id="eId-{}" onclick="processEventClick(this.id)">{}</a>'.format(
				codeLine[0],
				eventId,
				codeLine[1]
			);
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
					let label = "<a href='#'  id='eId-{}' onclick='processEventClick(this.id)' >{}</a>".format(
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
					let label = "<a href='#'  id='eId-{}' onclick='processEventClick(this.id)'>{}</a>".format(
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
	htmlProduced += '<div class="col-9 text-left dialog-title">Previous tests';
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
		let lblJump = "<a href='#'  id='eId-{}' onclick='processEventClick(this.id)'>{}</a>".format(test.id, 'View');

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
	htmlProduced += '<div class="col-9 text-left dialog-title">Condition breakdown';
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
				htmlProduced += '<a href="#" id="eId-{}" onclick="processEventClick(this.id)"><u>{}</u></a>'.format(
					foundEvent,
					sourceName
				);
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
				htmlProduced += 'Arithmetic expr.';
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
		htmlProduced += '<div class="row">';
		htmlProduced +=
			'<div class="col text-left mt-2" style="color:red; font-size:11pt;">Note:Source values repeated!! (less source indexes than targeted)';
		htmlProduced += '</div>';
		htmlProduced += '</div>';
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
function produceModalContent(eventId) {
	let content = {
		title: '',
		body: ''
	};
	eventId = eventId.split('-')[1];
	let event = events[eventId];

	switch (event['type']) {
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
		default:
			content.title = 'Ups...';
			content.body = 'Problem rendering event information';
	}

	return content;
}
function processEventClick(eventId) {
	let producedContent = produceModalContent(eventId);

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
