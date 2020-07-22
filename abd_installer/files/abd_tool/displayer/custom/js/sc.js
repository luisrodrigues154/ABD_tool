var data = {
	nodes: [
		{
			id: 'source1',
			x: 33,
			y: 133,
			width: 50,
			height: 50
		},
		{
			id: 'target1',
			x: 166,
			y: 66,
			width: 50,
			height: 50
		},
		{
			id: 'source2',
			x: 250,
			y: 40,
			width: 50,
			height: 50
		},
		{
			id: 'target2',
			x: 350,
			y: 133,
			width: 50,
			height: 50
		}
	],
	links: [
		{
			source: 'source1',
			target: 'target1',
			weight: 1,
			id: 'abc'
		},
		{
			source: 'source2',
			target: 'target2',
			weight: 3,
			id: 'xyz'
		}
	]
};

let svg = d3.select('svg').attr('width', 1200).attr('height', 500);

// nodes:

let nodes = svg
	.selectAll('.node')
	.data(data.nodes)
	.enter()
	.append('g')
	.attr('id', (d) => d.id)
	.attr('class', 'node')
	.attr('transform', (d) => 'translate(' + d.x + ',' + d.y + ')')
	.call(d3.drag().on('drag', dragged))
	.append('rect')
	.attr('width', 50)
	.attr('height', 50)
	.attr('fill', 'lime')
	.attr('rx', 5)
	.attr('ry', 5)
	.style('stroke', 'grey')
	.style('stroke-width', '1.5')
	.style('stroke-dasharray', '')
	.style('opacity', '.9')
	.style('cursor', 'pointer')
	.append('text')
	.attr('x', -12)
	.attr('y', 25)
	.attr('dx', 12)
	.attr('dy', '.35em')
	.text((d) => d.id)
	.style('cursor', 'pointer');

// links:

var pathMarker = svg
	.append('marker')
	.attr('id', 'pathMarkerHead')
	.attr('orient', 'auto')
	.attr('markerWidth', '8')
	.attr('markerHeight', '12')
	.attr('refX', '7')
	.attr('refY', '2')
	.append('path')
	.attr('d', 'M0,0 V4 L7,2 Z')
	.attr('fill', 'navy');

svg
	.selectAll('linkInGraph')
	.data(data.links)
	.enter()
	.append('path')
	.attr('class', 'linkInGraph')
	.attr('id', (d) => d.id)
	.attr('d', moveLink)
	.style('stroke-width', '2')
	.attr('stroke', 'blue')
	.attr('fill', 'none')
	.attr('marker-end', 'url(#pathMarkerHead)');

// drag behavior:

function dragged(n) {
	// Move the node:
	d3.select(this).attr('transform', (d) => 'translate(' + (d.x = d3.event.x) + ',' + (d.y = d3.event.y) + ')');

	// Move the link:
	d3.selectAll('.linkInGraph').filter((l) => l.source == n.id || l.target == n.id).attr('d', moveLink);
}

// link position:

function moveLink(l) {
	console.log('just trying');
	let filter = data.nodes.filter(function(n) {
		console.log(n);
		console.log('now L....');
		console.log(l);
		n.name == l.source;
	});
	console.log('now filter');
	console.log(filter);

	let nsid = data.nodes.filter((n) => n.id == l.source)[0].id;

	let ndid = data.nodes.filter((n) => n.id == l.target)[0].id;

	let ns = d3.select('#' + nsid).datum();
	let nd = d3.select('#' + ndid).datum();

	let min = Number.MAX_SAFE_INTEGER;
	let best = {};
	[ [ 25, 0 ], [ 50, 25 ], [ 25, 50 ], [ 0, 25 ] ].forEach((s) =>
		[ [ 25, 0 ], [ 50, 25 ], [ 25, 50 ], [ 0, 25 ] ].forEach((d) => {
			let dist = Math.hypot(nd.x + d[0] - (ns.x + s[0]), nd.y + d[1] - (ns.y + s[1]));
			if (dist < min) {
				min = dist;
				best = {
					s: { x: ns.x + s[0], y: ns.y + s[1] },
					d: { x: nd.x + d[0], y: nd.y + d[1] }
				};
			}
		})
	);

	var lineFunction = d3.line().x((d) => d.x).y((d) => d.y).curve(d3.curveLinear);

	return lineFunction([ best.s, best.d ]);
}
