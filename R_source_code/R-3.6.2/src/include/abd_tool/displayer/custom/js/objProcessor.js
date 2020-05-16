var cmnObj = objects['commonObj'];

//will not really be needed, cannot perform anything on code flow objects
var cfObj = objects['codeFlowObj'];

var trackObjects = [];

$(function() {
	populateObjects();
});

function clearObjectPane() {
	document.getElementById('objects_container').innerHTML = '';
}

function populateObjects() {
	var htmlProduced = '';
	var comObj = objects['commonObj'];
	var numObjs = Object.keys(comObj).length;
	var i;
	//alert('Num obj: ' + numObjs);
	for (i = 1; i <= numObjs; i++) {
		htmlProduced += '<div class="dropdown-item">';
		htmlProduced +=
			'<input type="checkbox" value="huehue" id="' + i + '" onChange="objTackStatusChanged(this.id);">';
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
	updateDisplay();
}

var wantDisplay = [];
var fakeBottomHr =
	'<hr class="mt-2 mb-3" style="visibility: hidden"/><hr class="mt-2 mb-3" style="visibility: hidden"/><hr class="mt-2 mb-3" style="visibility: hidden"/>';
var hr = '<hr class="mt-2 mb-3"/>';
var lhsLabel = '<label class="d-block object-lhs ml-2">';
var rhsLabel = '<label class="object-rhs-special ml-2">';
var tailLabel = '</label>';
function generateObjContent(objName, objId, objState, toIndex) {
	var htmlProduced = '';
	var currObjValue = getObjCurrValue(objId, objState, toIndex);
	//generate name row
	htmlProduced += lhsLabel;
	htmlProduced += 'Name: ';
	htmlProduced += rhsLabel;
	htmlProduced += objName;
	htmlProduced += tailLabel;
	htmlProduced += tailLabel;

	//generate struct type
	htmlProduced += lhsLabel;
	htmlProduced += 'Structure: ';
	htmlProduced += rhsLabel;
	htmlProduced += currObjValue[0];
	htmlProduced += tailLabel;
	htmlProduced += tailLabel;

	//generate data type
	htmlProduced += lhsLabel;
	htmlProduced += 'Data Type: ';
	htmlProduced += rhsLabel;
	htmlProduced += currObjValue[1];
	htmlProduced += tailLabel;
	htmlProduced += tailLabel;

	//generate current value
	htmlProduced += lhsLabel;
	htmlProduced += 'Current Value: ';
	htmlProduced += rhsLabel;
	htmlProduced += currObjValue[2];
	htmlProduced += tailLabel;
	htmlProduced += tailLabel;
	return htmlProduced;
}
function updateDisplay() {
	if (trackObjects.length == 0) return;
	var htmlProduced = '';
	var num = 0;
	var wantDisplayLen = wantDisplay.length;
	wantDisplay.forEach((obj) => {
		if (trackObjects.indexOf(String(obj.id)) > -1) {
			htmlProduced += generateObjContent(obj.name, obj.id, obj.state, obj.withIndex);
			htmlProduced += hr;
		}
	});
	htmlProduced += fakeBottomHr;

	document.getElementById('objects_container').innerHTML = htmlProduced;
}

function verifyTrack(objList) {
	wantDisplay = objList;
	updateDisplay();
}

function getObjCurrValue(id, state, index) {
	var currentValue = [];
	cmnObj[id]['modList'].forEach((mod) => {
		//resolve

		if (mod['id'] == state) {
			if (mod['vecMod'] == false) {
				currentValue.push(mod['structType']);
				currentValue.push(mod['dataType']);
				currentValue.push(mod['value']);
				return currentValue;
			}
		}
	});
	return currentValue;
}

function clearWantDisplay() {
	wantDisplay = [];
	clearPanes();
}

function getObjById(id) {
	return cmnObj[id];
}

function getObjNameById(id) {
	return cmnObj[id]['name'];
}
