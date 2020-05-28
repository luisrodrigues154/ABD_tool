var options = {
	manipulation: false,
	height: '90%',
	layout: {
		hierarchical: {
			enabled: true,
			levelSeparation: 350
		}
	},
	physics: {
		hierarchicalRepulsion: {
			nodeDistance: 350
		}
	},
	edges: {
		smooth: {
			type: 'cubicBezier',
			roundness: 1
		}
	},
	nodes: {
		font: {
			// required: enables displaying <b>text</b> in the label as bold text
			multi: 'html',
			// optional: use this if you want to specify the font of bold text
			bold: '16px arial black'
		}
	}
};
var stack = [];
var stackSize = 0;
var currentEnv = '';
var blocks = new Map();
let breakLine = '\n';
let tab = '    ';
let tab2 = '   ';
let eventsLen = 0;
var nodes = [];
var edges = [];

//d3 variable
var graph = {
	nodes: [
		{
			name: 'ABS',
			id: 'id-1',
			html:
				'<node id="node-id-1" ><label id="1" onclick="alert(this.id)">Node 1</label><label id="1" onclick="alert(this.id)">Node 1</label><label id="1" onclick="alert(this.id)">Node 1</label><label id="1" onclick="alert(this.id)">Node 1</label><label id="1" onclick="alert(this.id)">Node 1</label><label id="1" onclick="alert(this.id)">Node 1</label><label id="1" onclick="alert(this.id)">Node 1</label></node>'
		},
		{
			name: 'ATS',
			id: 'id-2',
			html:
				'<node id="node-id-2"><label id="2" onclick="alert(this.id)">Node 2</label><label id="1" onclick="alert(this.id)">Node 1</label><label id="1" onclick="alert(this.id)">Node 1</label><label id="1" onclick="alert(this.id)">Node 1</label><label id="1" onclick="alert(this.id)">Node 1</label></node>'
		},
		{
			name: 'ATS222',
			id: 'id-3',
			html:
				'<node id="node-id-3"><label id="3" onclick="alert(this.id)">Node 3</label><label id="3" onclick="alert(this.id)">Node 3</label><label id="3" onclick="alert(this.id)">Node 3</label></node>'
		}
	],
	links: [ { source: 'ABS', target: 'ATS', id: 1 }, { source: 'ATS', target: 'ATS222', id: 2 } ]
};
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

$(function() {
	//load function
	//buildNodes();
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
	processEnv('main', events[1]['atEnv'], 1);
}

function processEnv(funcName, env, idxStart) {
	let i;
	pushToStack(env);
	let blockHead = blockHeader(funcName, currentEnv);
	let blockStr = blockHead;
	let log = 'funcName: {}, idxStart: {}'.format(funcName, idxStart);
	console.log('will execute with: ' + log);
	let lastLine = 0;
	for (i = idxStart; i <= eventsLen; i++) {
		if (events[i]['atEnv'] == env) {
			blockStr += '{}{}'.format(getStrForEventType(events[i], lastLine, i + 1), breakLine);
			lastLine = events[i]['line'];
		}
	}
	blocks.set(env, blockStr);
	popFromStack();
}

function getStrForEventType(event, lastLine, idxStart) {
	switch (event['type']) {
		case types.ASSIGN:
			return getStrForAssign(event, lastLine);
		case types.IF:
			return '';
		case types.FUNC:
			let newFuncName = getCodeFlowObjNameById(event['data']['toId']);
			processEnv(newFuncName, event['data']['toEnv'], idxStart);
			addEdge(event['atEnv'], event['data']['toEnv']);
			return getStrForFunction(event, lastLine);
		case types.RET:
			return '';
		case types.ARITH:
			return '';
		case types.VEC:
			return '';
		default:
			break;
	}
}
function getStrForFunction(event, lastLine) {
	let producedStr = '';
	let line = event['line'];

	if (line != lastLine) producedStr += '{}:Function Call{}'.format(line, breakLine);
	else producedStr += '{}Function Call{}'.format(tab2, breakLine);

	return producedStr;
}
function getStrForAssign(event, lastLine) {
	let producedStr = '';
	let line = event['line'];
	let destId = event['data']['toObj'];
	let originText = '';

	if (line != lastLine) producedStr += '{}:Assignment{}'.format(line, breakLine);
	else producedStr += '{}Assignment{}'.format(tab2, breakLine);

	producedStr += '{}To:{}{}'.format(tab, getCommonObjNameById(destId), breakLine);

	if (event['data']['origin'] == 'obj') {
		if (event['data']['fromObj'] == 'HC') {
			originText = HC_text;
		} else if (event['data']['fromObj'] == 'R') {
			originText = R_text;
		} else {
			originText = getCommonObjNameById(event['data']['fromId']);
		}
		producedStr += '{}From:{}{}'.format(tab, originText, breakLine);
	} else {
		// var event = getEventById(event['data']['fromEvent']);
		// processMe = 1;
		// eventContent += processEventByType(event);
		producedStr += '{}From:{}{}'.format(tab, 'event', breakLine);
	}

	return producedStr;
}

function blockHeader(func_name, env) {
	let nodeStr = 'Function:{}(){}'.format(func_name, breakLine);
	nodeStr += 'Env:{}{}'.format(env, breakLine);
	nodeStr += '{}{}'.format('---------------------------------', breakLine);
	return nodeStr;
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
function gravity(alpha) {
	return function(d) {
		d.y += (d.cy - d.y) * alpha;
		d.x += (d.cx - d.x) * alpha;
	};
}

function generateSVGgraph() {
	nodeSizeScale = d3.scaleLinear().domain(d3.extent(graph.nodes, (d) => d.id)).range([ 30, 30 ]);

	simulation = d3
		.forceSimulation()
		.force('link', d3.forceLink().id((d) => d.id))
		.force('charge', d3.forceManyBody())
		.force('center', d3.forceCenter(100, 100))
		.nodes(graph.nodes);

	var link_force = d3.forceLink(graph.links).id(function(d) {
		return d.name;
	});

	charge_force = d3.forceManyBody().strength(-30000);

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
	g = svg.append('g').attr('class', 'everything');

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
		.attr('id', (d) => d.id);

	drag_handler = d3.drag().on('start', drag_start).on('drag', drag_drag);
	drag_handler(nodes);

	console.log(g);

	graph.nodes.forEach((node) => {
		document.getElementById(node.id).innerHTML = node.html;
	});

	var zoom_handler = d3.zoom().on('zoom', zoom_actions);

	zoom_handler(svg);

	graph.links.forEach(function(d) {
		linkedByIndex[d.source.index + ',' + d.target.index] = 1;
	});
	g.transition().duration(400).call(zoom_handler.transform, d3.zoomIdentity);
}
function zoom_actions() {
	g.attr('transform', d3.event.transform);
}
function validate(x, a, b) {
	if (x < a) x = a;
	if (x > b) x = b;
	return x;
}
function drag_start(d) {
	if (!d3.event.active) simulation.alphaTarget(0.3).restart();
	d3.select(this).classed('fixed', (d.fixed = true));
}

//make sure you can't drag the circle outside the box
function drag_drag(d) {
	d.fx = validate(d3.event.x, 0, width);
	d.fy = validate(d3.event.y, 0, height);
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
}

function positionLink1(d) {
	var dr = -300;
	let min = Number.MAX_SAFE_INTEGER;
	let best = {
		s: { x: 0, y: 0 },
		d: { x: 0, y: 0 }
	};
	let width = document.getElementById('node-' + d.source.id).offsetWidth + 10;
	let sourceHeight = document.getElementById('node-' + d.source.id).offsetHeight;
	let targetHeight = document.getElementById('node-' + d.target.id).offsetHeight;
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
