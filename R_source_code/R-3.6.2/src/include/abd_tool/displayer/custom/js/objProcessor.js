var cmnObj = objects['commonObj'];

//will not really be needed, cannot perform anything on code flow objects
var cfObj = objects['codeFlowObj'];

var trackObjects = [];

$(function() {
	populateObjects();
});

function populateObjects() {
	var htmlProduced = '';
	var comObj = objects['commonObj'];
	var numObjs = Object.keys(comObj).length;
	var i;
	//alert('Num obj: ' + numObjs);
	for (i = 1; i <= numObjs; i++) {
		htmlProduced += '<div class="dropdown-item">';
		htmlProduced +=
			'<input type="checkbox" value="huehue" id="' + i + ' " onChange="objTackStatusChanged(this.id);">';
		htmlProduced += '<label class="ml-2">' + comObj[i]['name'] + '</label>';
		htmlProduced += '</div>';
	}

	document.getElementById('objDropDown').innerHTML = htmlProduced;
}

function objTackStatusChanged(id) {
	var ret = trackObjects.indexOf(id);

	if (ret > -1)
		//remove
		trackObjects.splice(ret, 1);
	else
		//add
		trackObjects.push(id);

	//here update the object view
}
