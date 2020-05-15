const pathTree = new Set();
const visiblePathTree = new Set().add('/0');
let cachedVisiblePathTree = new Set();

$(function () {
    // Add all known paths.
    $('div[data-path]').each(function () {
        const path = $(this).data('path');

        pathTree.add(path);
        if (visiblePathTree.has(path)) showPath(path);
        else hidePathNode($(this));
    });

    // Toggle pane visibility on click.
    $('button[data-path]').on('click', function () {
        const path = $(this).data('path');

        if (visiblePath('/1') && !path.startsWith('/1')) return false;
        if (path.startsWith('/1')) {
            if (visiblePath('/1')) {
                // Show paths from cache.
                for (const _path of cachedVisiblePathTree) {
                    showPath(_path);
                }
            } else {
                cachedVisiblePathTree = new Set(visiblePathTree);
                hidePath('/2');
            }

            // Remove the main pane from the cache.
            visiblePathTree.delete('/0');
        }

        if (visiblePath(path)) hidePath(path);
        else showPath(path);

        if (visiblePathTree.size !== 0) hidePath('/0');
        else showPath('/0');

        if (visiblePath('/2/2/1') && visiblePath('/2/2/2')) {
            // Force max size.
            getPathNode('/2/2/1').addClass('limit-max-height');

        } else getPathNode('/2/2/1').removeClass('limit-max-height');
    });
});

function visiblePath(path) {
    return visiblePathTree.has(path);
}

function getPathNode(path) {
    return $(`div[data-path="${path}"]`);
}

function showPathNode(node) {
    node.removeClass('d-none');
}

function hidePathNode(node) {
    node.addClass('d-none');
}

function showPath(path) {
    showPathNode(getPathNode(path));
    visiblePathTree.add(path);

    for (const _path of pathPredecessors(path)) {
        showPathNode(getPathNode(_path));
        visiblePathTree.add(_path);
    }
}

function hidePath(path) {
    hidePathNode(getPathNode(path));
    visiblePathTree.delete(path);

    for (const _path of pathSuccessors(path)) {
        hidePathNode(getPathNode(_path));
        visiblePathTree.delete(_path);
    }

    // What if my predecessor has no visible successors.
    const predecessors = pathPredecessors(path);
    let predecessor;

    for (let i = predecessors.length - 1; i >= 0; i--) {
        predecessor = predecessors[i];
        if (!visiblePathSuccessors(predecessor)) {
            hidePathNode(getPathNode(predecessor));
            visiblePathTree.delete(predecessor);
        } else break;
    }
}

function pathPredecessors(path) {
    const splitPath = path.split('/');
    const result = [];
    let currentPath = '';

    for (let i = 1; i < splitPath.length - 1; i++) {
        currentPath += '/' + splitPath[i];
        result.push(currentPath);
    }

    return result;
}

function pathSuccessors(path) {
    const result = [];
    for (const _path of pathTree) {
        if (_path !== path && _path.startsWith(path)) {
            result.push(_path);
        }
    }

    return result;
}

function visiblePathSuccessors(path) {
    for (const _path of pathSuccessors(path)) {
        if (visiblePath(_path)) return true;
    }

    return false;
}

function checkFileApi() {
    return window.File && window.FileReader && window.FileList && window.Blob;
}

function readTextFile(file) {
    if (!checkFileApi()) alert('File API not supported.');

    const rawFile = new XMLHttpRequest();
    rawFile.open('GET', file, false);
    rawFile.onreadystatechange = function () {
        if (rawFile.readyState === 4) {
            if (rawFile.status === 200 || rawFile.status === 0) {
                const allText = rawFile.responseText;
                alert(allText);
            }
        }
    };
    rawFile.send(null);
}
