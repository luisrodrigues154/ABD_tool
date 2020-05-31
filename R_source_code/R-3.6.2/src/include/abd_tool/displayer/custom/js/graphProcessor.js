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

$(function() {
	//load function
	buildNodes();

	/*console.log('nodes');
	console.log(graph.nodes);
	console.log('----------------');
	console.log('links');
	console.log(graph.links);*/
	generateSVGgraph();
});

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
	for ([ line, content ] of envContent.get(env)) {
		var numEntries = content.length - 1;
		var i;
		let auxHtml = '';
		for (i = numEntries; i >= 0; i--) {
			auxHtml += '{} {} '.format(content[i], i > 0 ? '<-' : '');
		}
		nodeHtml += genNodeRow(line, auxHtml);
	}

	nodeHtml += '</div>';
	nodeHtml += '</node>';
	addNodeToDict(env, nodeHtml);
	//console.log(graph);
}

function addNodeToDict(env, html) {
	graph.nodes.push({ name: env, id: ++nodeCount, html: html });
}

function initEnvMap(env) {
	envContent.set(env, new Map());
}
function processEnv(funcName, env, idxStart, calledFromLine) {
	let i;
	pushToStack(env);
	initEnvMap(env);

	let lastLine = 0;
	for (i = idxStart; i <= eventsLen; i++) {
		if (events[i]['atEnv'] == env) {
			let htmlRcvd = getEventTypeHtml(events[i], i + 1);
			lastLine = events[i]['line'];
			addEventToEnvMap(env, lastLine, htmlRcvd);
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
	htmlProduced += '<label class="float-right object-rhs-special">Function:</line>';
	htmlProduced += '</div>';
	htmlProduced += '<div class="col col-6">';
	htmlProduced += '<label>' + funcName.trim() + '()</label>';
	htmlProduced += '</div>';
	htmlProduced += '</div>';

	//environment

	htmlProduced += '<div class="row">';
	htmlProduced += '<div class="col col-5">';
	htmlProduced += '<label class="float-right object-rhs-special">Environment:</line>';
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
		htmlProduced += '<label class="float-right object-rhs-special">From Line:</line>';
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
function genNodeRow(line, codeHtml) {
	var htmlProduced = '<div class="row">';

	//line of code
	htmlProduced += '<div class="col col-lg-2 col1">';
	htmlProduced += '<label class="float-right object-rhs-special">' + line + '</label>';
	htmlProduced += '</div>';

	//code
	htmlProduced += '<div class="col col-md-auto col3">';
	htmlProduced += '<codeWrapper>' + codeHtml + '</codeWrapper>';
	htmlProduced += '</div>';

	//terminators
	htmlProduced += '</div>';

	return htmlProduced;
}

function addEventToEnvMap(env, line, eventContent) {
	if (!envContent.get(env).has(line)) envContent.get(env).set(line, []);

	envContent.get(env).get(line).push(eventContent);
}
function genLabelHtml(id, text) {
	return "<label id='{}' onclick='alert(this.id)'>{}</label>".format(id, text);
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
					.trim()
			);

			break;
		case types.ASSIGN:
			let objId = event['data']['toObj'];
			let obj = getCommonObjNameById(objId);
			let state = event['data']['toState'];
			if (event['data']['origin'] == 'obj') {
				htmlProduced += genLabelHtml('eId-{}'.format(nextEventId - 1, objId, state), codeLine.trim());
			} else {
				htmlProduced += genLabelHtml(
					'eId-{}'.format(nextEventId - 1, objId, state),
					codeLine.substring(codeLine.indexOf(obj), codeLine.indexOf('<-', codeLine.indexOf(obj))).trim()
				);
			}
			break;
		case types.IF:
			break;
		case types.RET:
			htmlProduced += "<ret id='eId-{}' onclick='alert(this.id)'> {} </ret>".format(nextEventId - 1, '(return)');
			break;
		case types.ARITH:
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
	console.log(stack);
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
		(best.d.y == d.target.y ? best.d.y - 20 : best.d.y + 10)
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
