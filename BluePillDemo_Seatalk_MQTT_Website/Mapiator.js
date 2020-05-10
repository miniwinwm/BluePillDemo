var Mapiator = {};
Mapiator.util = {
	modulo: function(val, mod){
		var res = val%mod;
		if(res < 0) res += mod;
		return res;
	},
	byId: function(str) {
		return document.getElementById(str);
	},
	copy: function(obj, c) {
		if(!c) c = {};
		for( k in obj ) c[k] = obj[k];
		return c;
	},
	clone: function(obj) {
		function Constructor(){}
		Constructor.prototype = obj;
		return new Constructor();
	},
	forEach: function(array, fun) {
		for(var i=0; i<array.length; i++)
			fun(array[i], i);
	},
	mapArray: function(array, fun) {
		var res = [];
		for(var i=0; i<array.length; i++)
			res[i] = fun(array[i]);
		return res;
	},
	addCssRule: function(name, css, callback) {
		var ss = document.styleSheets[0];
		if (ss.addRule) { // Browser is IE?
			var selectors = name.split(',');
			for( i in selectors ) ss.addRule(selectors[i], css, 0);
	    } else ss.insertRule(name+'{'+css+'}', 0);
	},
	gudermann: function(y) {
		return 2*Math.atan(Math.exp(y)) - 0.5*Math.PI
	},
	inverseGudermann: function(rho) { // inverse of the Gudermannian function
		return Math.log(Math.tan(Math.PI*0.25 + rho*0.5));
	},
	project: function(lat, lng){
		var rho = (lat*Math.PI/180.0);
		return [lng/360.0, -0.5*Mapiator.util.inverseGudermann(rho)/Math.PI];
	},
	inverseProject: function(x, y) {
		var yScaled = -2*Math.PI*y;
		return [180*Mapiator.util.gudermann(yScaled)/Math.PI, x*360];
	},
	pixelCoordinates: function( lat, lng, mapExtendInPx ) {
		var p = Mapiator.util.project(lat, lng);
		return [
				Math.floor( (p[0]+0.5)*mapExtendInPx ),
				Math.floor( (p[1]+0.5)*mapExtendInPx )
		];
	},
	MovableContainer: function( parentEl ) {
		var div = document.createElement('div');
		div.style.position = 'absolute';
		
		this.move = function( x,y ) {
			this.offsetX += x;
			this.offsetY += y;
			div.style.left = ''+ this.offsetX +'px';
			div.style.top  = ''+ this.offsetY +'px';
		};
		
		this.reposition = function( mox, moy ) {
			this.offsetX = 0;
			this.offsetY = 0;
			this.move(0,0);
			
			this.offsetToLeftMapBorder = mox;
			this.offsetToTopMapBorder = moy;
		};
		
		this.appendChild = function( c ) {
			div.appendChild( c );
		};
		
		this.removeChild = function( c ) {
			div.removeChild( c );
		};
		
		this.reposition();
		parentEl.appendChild( div );
	},
	VisibleArea: function(lat,lng, mapExtendInPx, vpWidth, vpHeight) {
		var util = Mapiator.util;
		var p = util.project(lat, lng);
		
		this.left = Math.round( p[0]*mapExtendInPx + 0.5*(mapExtendInPx - vpWidth) );
		this.top = Math.round( p[1]*mapExtendInPx + 0.5*(mapExtendInPx - vpHeight) );
		this.width = vpWidth;
		this.height = vpHeight;
		this.move = function(mx,my) {
			this.left += mx;
			this.top += my;
			this.right = this.left + this.width;
			this.bottom = this.top + this.height;
		};
		this.centerX = function() {
			return this.left + 0.5*vpWidth;
		};
		this.centerY = function() {
			return this.top + 0.5*vpHeight;
		};
		this.centerLatLng = function() {
			return util.inverseProject(
				(this.centerX()-0.5*mapExtendInPx)/mapExtendInPx,
				(this.centerY()-0.5*mapExtendInPx)/mapExtendInPx
			);
		};
		this.latLngAt = function(pX, pY) {
			return util.inverseProject(
				(this.left + pX - 0.5*mapExtendInPx)/mapExtendInPx,
				(this.top + pY - 0.5*mapExtendInPx)/mapExtendInPx
			);
		};
		this.move(0,0);
	}
};


Mapiator.CanvasTile = function(x, y, zoom, ox, oy, map) {
	var util = Mapiator.util;
	
	this.zoom = zoom;

	this.x = x;
	this.y = y;
	this.offsetX = ox;
	this.offsetY = oy;
	
	this['id'] = Mapiator.CanvasTile.idFor(x,y,zoom);

	var tilesPerRow = (1<<zoom);
	// check if any of the paths' and polygons' bounding box overlaps this tile:
	var drawTile = false;
	for( id in map._pathsAndPolygons) {
		var pathOrPolygon = map._pathsAndPolygons[id];
		var l = pathOrPolygon.bbLeft+0.5, t = pathOrPolygon.bbTop + 0.5, r = pathOrPolygon.bbRight+0.5, b = pathOrPolygon.bbBottom+0.5;
		var tl = x/tilesPerRow,           tt = y/tilesPerRow,            tr = (x+1)/tilesPerRow,        tb = (y+1)/tilesPerRow; 
		if( l<=tr && r>=tl && t<=tb && b>=tt ) {
			drawTile = true;
			break;
		}
	}
	if( !drawTile ) return;
	
	var el = document.createElement('canvas');
	el.width = map.tileSizeInPx;
	el.height = map.tileSizeInPx;
	el.id = this['id'];
	
	// we need to init this for IE
	if (typeof G_vmlCanvasManager != "undefined") {
	  	// temporarily add the element to the dom
		document.body.appendChild(el);
		G_vmlCanvasManager.initElement(el);
		el = null;
		// get the replacement:
		this.domElement = util.byId(this['id']);
		// and remove it again:
		document.body.removeChild(this.domElement);
	}
	else this.domElement = el;
	
	var s = this.domElement.style;
	s.position = 'absolute';
	s.zIndex = '5';
	s.left = ''+ this.offsetX +'px';
 	s.top = ''+ this.offsetY +'px';
	// s.backgroundImage = 'url('+ this.url +')';
	
	var ctx = this.domElement.getContext('2d');
	
	ctx.lineWidth = 1.0/map.tileSizeInPx;
	ctx.scale(map.tileSizeInPx, map.tileSizeInPx);
	ctx.translate( -this.x, -this.y );
	ctx.lineWidth /= tilesPerRow;
	ctx.scale(tilesPerRow, tilesPerRow);
	ctx.translate( 0.5, 0.5 );
	
	function drawElements() {
		for( id in map._pathsAndPolygons) {
			var pathOrPolygon = map._pathsAndPolygons[id];
			var isPath = (pathOrPolygon.type == 'Path');
			ctx.save();
				if( isPath ) {
					ctx.strokeStyle = pathOrPolygon.strokeStyle;
					ctx.lineWidth *= pathOrPolygon.strokeWidth;
				}
				else // it's a polygon 
					ctx.fillStyle = pathOrPolygon.fillStyle;
					
				ctx.beginPath();
				util.forEach( pathOrPolygon.projectedPoints, function(p, index) {
					if( index == 0 ) ctx.moveTo(p[0], p[1]);
					else ctx.lineTo(p[0], p[1]);
				});
				if( isPath ) { ctx.stroke(); }
				else ctx.fill();
			ctx.restore();
			
			// for debuging: draw bounding box:
			// ctx.beginPath();
			// 	ctx.moveTo(
			// 		pathOrPolygon.bbLeft,
			// 		pathOrPolygon.bbTop);
			// 	ctx.lineTo(pathOrPolygon.bbRight, pathOrPolygon.bbTop);
			// 	ctx.lineTo(pathOrPolygon.bbRight, pathOrPolygon.bbBottom);
			// 	ctx.lineTo(pathOrPolygon.bbLeft, pathOrPolygon.bbBottom);
			// 	ctx.lineTo(pathOrPolygon.bbLeft, pathOrPolygon.bbTop);
			// ctx.stroke();	
		}
	}
	
	drawElements();
};
Mapiator.CanvasTile.idFor = function(x,y,zoom){
	return 'canvas_tile_'+ x + '_' + y + '_' + zoom;
};


Mapiator.StdTile = function(x, y, zoom, ox, oy, map) {
	var util = Mapiator.util;
	
	this.zoom = zoom;

	this.x = x;
	this.y = y;
	this.offsetX = ox;
	this.offsetY = oy;
	
	this['id'] = Mapiator.StdTile.idFor(x,y,zoom);
	this.url = map.getTileUrl(this.x,this.y,zoom);

	this.domElement = document.createElement('div');
	this.domElement.id = this['id'];
	var s = this.domElement.style;
	s.position = 'absolute';
	s.width = '' + map.tileSizeInPx + 'px';
	s.height = '' + map.tileSizeInPx + 'px';
	s.zIndex = '1';
	s.left = ''+ this.offsetX +'px';
 	s.top = ''+ this.offsetY +'px';
	s.background = 'url('+ this.url +')';
	
	// for debuging:
	// this.domElement.innerHTML = this['id'];
};
Mapiator.StdTile.idFor = function(x,y,zoom){
	return 'std_tile_'+ x + '_' + y + '_' + zoom;
};

Mapiator.OverlayLayer = function( map ){
	var util = Mapiator.util;
	
	var containerDiv;
	var overlays = [];
	
	function displayOverlay( o ) {
		var p = util.pixelCoordinates(o.lat,o.lng, map.mapExtendInPx);
		var x = p[0] - map.movableContainer.offsetToLeftMapBorder;
		var y = p[1] - map.movableContainer.offsetToTopMapBorder;
		
		var s = o.element.style;
		s.position = 'absolute';
		s.left = ''+ x +'px';
		s.top = ''+ y +'px';
		containerDiv.appendChild(o.element);
	}
	
	this.addElement = function(el, lat, lng) {
		var o = {element:el, lat:lat, lng:lng};
		overlays[overlays.length] = o;
		
		if(map.movableContainer.offsetToMapLeftBorder) displayOverlay( o );
	};
	
	this.redraw = function() {
		if( containerDiv ) map.movableContainer.removeChild( containerDiv );
		containerDiv = document.createElement('div');
		var s = containerDiv.style;
		s.position = 'absolute';
		s.left = '0';
		s.top = '0';
		s.zIndex = '20';
		map.movableContainer.appendChild( containerDiv );

		util.forEach(overlays, displayOverlay);
	};
}

Mapiator.TileLayer = function(map, visibleArea, TileConstructor){
	// tile layer expects map.movableContainer to be at the upper left corner of
	// the visibleArea on creation

	var util = Mapiator.util;
	
	var tileCache = {};
	function findTile(x,y,zoom){
		return tileCache[TileConstructor.idFor(x,y,zoom)];
	};
	
	
	
	function tileAtPosition(x,y){
		return [Math.floor(x/map.tileSizeInPx), Math.floor(y/map.tileSizeInPx)];
	}
	
	var offsetX = visibleArea.left % map.tileSizeInPx;
	var offsetY = visibleArea.top % map.tileSizeInPx;
	
	var baseTilePos = tileAtPosition(visibleArea.left, visibleArea.top);
	
	var tileContainer = document.createElement('div');
	var s = tileContainer.style;
	s.position = 'absolute';
	s.left = '-'+ offsetX + 'px';
	s.top = '-'+ offsetY + 'px';
	s.zIndex = '0';
	// for debuging:
	// s.width = ''+visibleArea.width+'px';
	// s.height = ''+visibleArea.height+'px';
	// s.backgroundColor = '#477';
	map.movableContainer.appendChild( tileContainer );
		
	//alert('vb.centerX()='+visibleArea.centerX());
	
	//alert('ll='+visibleArea.centerLatLng());
	
	function removeAllTilesNotContainedIn( hash ) {
		for( id in tileCache ){
			if( !hash[id] ){
				var tile = tileCache[id];
				if( tile.domElement ) tileContainer.removeChild( tile.domElement );
				delete tileCache[id];
			}
		}
	}

	
	this.showTiles = function() {
		// alert('TileLayer#showTiles');
		var topLeftTileNo = tileAtPosition(visibleArea.left, visibleArea.top);
		var bottomRightTileNo = tileAtPosition(visibleArea.right, visibleArea.bottom);
		
		var tiles = {};
		for(var x=topLeftTileNo[0]; x <= bottomRightTileNo[0]; ++x ){
			for(var y=topLeftTileNo[1]; y <= bottomRightTileNo[1]; ++y ){
				var tile = findTile(x,y,map.zoom);
				if( !tile ) {
					var ox = (x - baseTilePos[0]) * map.tileSizeInPx;
					var oy = (y - baseTilePos[1]) * map.tileSizeInPx;
					tile = new TileConstructor(x, y, map.zoom, ox, oy, map);
					tileCache[tile['id']] = tile;
					if( tile.domElement ) tileContainer.appendChild( tile.domElement );
				}
				tiles[tile['id']] = tile;
			}
		}
		removeAllTilesNotContainedIn( tiles );
	}
	
	this.showTiles();
	
	this.destroy = function() {
		map.movableContainer.removeChild( tileContainer );
	}
};

Mapiator.PathOrPolygon = function( points ) {
	this['id'] = Mapiator.PathOrPolygon._nextID++;
    this.points = points || [];
	this.appendPoint = function( lat, lng ) {
		this.points[this.points.length] = [lat,lng];
	};
	this.recalc = function() {
		this.projectedPoints = [];
		var l,t,r,b;
		var p = this.points;
		for(var i=0; i < p.length; ++i){
			var pp = this.projectedPoints[i] = Mapiator.util.project(p[i][0], p[i][1]);
			l = (l && l<pp[0]) ? l : pp[0];
			t = (t && t<pp[1]) ? t : pp[1];
			r = (r && r>pp[0]) ? r : pp[0];
			b = (b && b>pp[1]) ? b : pp[1];				
		}
		this.bbLeft = l;
		this.bbTop = t;
		this.bbRight = r;
		this.bbBottom = b;
		// console.log('l=',l,'t=',t,'r=',r,'b=',b);
	};
	this.recalc();
};
Mapiator.PathOrPolygon._nextID = 1;

Mapiator.Path = function( points ) {
	Mapiator.PathOrPolygon.call( this, points );
}
Mapiator.Path.prototype = {
	type: 'Path',
	strokeStyle: "rgba(0,0,0, 1.0)",
	strokeWidth: 2.0
};

Mapiator.Polygon = function( points ) {
	Mapiator.PathOrPolygon.call( this, points );
}
Mapiator.Polygon.prototype = {
	type: 'Polygon',
	// strokeStyle: "rgba(0,0,0, 1.0)",
	fillStyle: "rgba(0,0,0, 1.0)"
};

Mapiator.parseWKT = function( wkt ) {
	var Point = function( lat,lng ) {
		this['id'] = Mapiator.PathOrPolygon._nextID++;
        this.lat = lat;
        this.lng = lng;
    };
	Point.prototype = {type:'POINT'};

    regex = {
        point: /^\s*POINT\s*\(\s*(-?\d+(?:\.\d+)?)\s+(-?\d+(?:\.\d+)?)\s*\)\s*$/i, // POINT (11.5757639569603175 48.1365236143369373)
        lineString: /^LINESTRING\s*\((.*)\)\s*$/i, // LINESTRING (11.5752 48.1372, 11.5750 48.1376, 11.5756 48.1371)
		polygon: /^POLYGON\s*\((.*)\)\s*$/i // POLYGON (11.5752 48.1372, 11.5750 48.1376, 11.5756 48.1371)
    };

	var m;
	if( m = regex.point.exec( wkt ) ){
		return new Point( parseFloat(m[2]), parseFloat(m[1]) );
	}
	else {
		var p;
		if( m = regex.lineString.exec( wkt ) ){
			p = new Mapiator.Path();
        }
		else if( m = regex.polygon.exec( wkt ) ){
			p = new Mapiator.Polygon();
		}
		else return null;
		
		var pointStrings = m[1].split(/\s*,\s*/);
		Mapiator.util.forEach( pointStrings, function(ps){
			var m2;
			if( m2 = /^\s*(-?\d+(?:\.\d+)?)\s+(-?\d+(?:\.\d+)?)\s*$/.exec(ps) )
				p.appendPoint( parseFloat(m2[2]), parseFloat(m2[1]) );
		});
		p.recalc();
		return p;
	}
};

Mapiator.Map = function( divId ) {
	var util = Mapiator.util;
	var self = this;
	
	var IE='\v'=='v'; // detect IE
	this.IE = IE;
	var ua = navigator.userAgent;
	this.iPhone = ua.match(/iPhone/i) || ua.match(/iPod/i);
	
	this.tileSizeInPx;
	this.maxZoom = 18;
	this.minZoom = 0;
	
	this.mapDiv = util.byId( divId );
	this.mapDiv.style.overflow = "hidden";
	this.movableContainer = new util.MovableContainer( this.mapDiv );
	var visibleArea;

	var centerLat, centerLng;
	this.setZoomLevel = function( level, pX, pY ) {
		if( visibleArea ){
			// we are changing the zoom level but the map may have been panned around
			// but the current center is only stored in pixel coordinates which are different
			// for every zoom level. Therefor we recalculate the centerLat and centerLng:
			var ll = pX ? visibleArea.latLngAt( pX, pY ) : visibleArea.centerLatLng();
			centerLat = ll[0];
			centerLng = ll[1];
			visibleArea = null;
		}
		this.zoom = level;
		this.mapExtendInPx = this.tileSizeInPx * (1<<this.zoom);
	};
	// pX and pY are optional pixel coordinates. If set they
	// define the center of the map after the zoom. Otherwise
	// the center will be the same as before the zoom
	this.zoomIn = function(pX, pY) {
		if( this.zoom >= this.maxZoom ) return;
		this.setZoomLevel( this.zoom + 1, pX, pY);
		this.redraw();
	};
	this.zoomOut = function() {
		if( this.zoom <= this.minZoom ) return;
		this.setZoomLevel( this.zoom - 1);
		this.redraw();
	};
	
	this.setTileSizeInPx = function( size ) {
		this.tileSizeInPx = size;
		this.mapExtendInPx = this.tileSizeInPx * (1<<this.zoom);
	};
	this.setTileSizeInPx(256);
	
	this.setCenter = function(lat,lng){
		centerLat = lat;
		centerLng = lng;
	};
	
	this.getTileUrl = function(x, y, zoom){
		// return 'tiles/'+ zoom +'_'+ x +'-'+ y +'.png';
		if( zoom >= 12 ) return 'http://b.tile.openstreetmap.org/'+zoom+'/'+x+'/'+y+'.png';
		return 'http://maps-for-free.com/layer/relief/z'+ zoom +'/row'+ y +'/'+ zoom +'_'+ x +'-'+ y +'.jpg';
	};
	
	// -----------------------------------------------------
	this._pathsAndPolygons = {};
	this._projectedPoints = {};
	this.addElement = function( p ) {
		this._pathsAndPolygons[p['id']] = p;
	};
	this.removeElement = function( p ) {
		delete this._pathsAndPolygons[p['id']];
	};
	
	this.moveByPx = function( x,y ) {
		this.movableContainer.move( x,y );
		// the visible area is relative to the map
		visibleArea.move( -x,-y );
		
		this.tileLayer.showTiles();
		if(!IE) this.canvasTileLayer.showTiles();
	};
	
	this.redraw = function() {
		
		// deprecated:
		this.width = this.mapDiv.offsetWidth;
		this.height= this.mapDiv.offsetHeight;
		
		visibleArea = new util.VisibleArea(centerLat, centerLng, this.mapExtendInPx, this.width, this.height);
		this.movableContainer.reposition( visibleArea.left, visibleArea.top );
		
		if( this.tileLayer ) this.tileLayer.destroy();		
		this.tileLayer = new Mapiator.TileLayer(this, util.clone(visibleArea), Mapiator.StdTile);
		
		if(!window.debugPD) {
			if( this.canvasTileLayer ) this.canvasTileLayer.destroy();
			this.canvasTileLayer = new Mapiator.TileLayer(this, util.clone(visibleArea), Mapiator.CanvasTile);
		}
			
		this.overlayLayer.redraw();
	};
	
	this.overlayLayer = new Mapiator.OverlayLayer(this);
	if( this.iPhone ) Mapiator.iPhoneController( this );
	else Mapiator.TraditionalController( this );
};

Mapiator.TraditionalController = function( map ) {
	var util = Mapiator.util;
	
	// panning:
	var xmove, ymove;
	function moveMap(e) {
		if( map.IE ) e = window.event;
		else e.preventDefault();
		if( typeof xmove != 'undefined' )
			map.moveByPx( e.clientX - xmove, e.clientY - ymove );
		xmove = e.clientX;
		ymove = e.clientY;
	}
	function disableDrag(e){
		if( map.IE ) e = window.event;
		else e.preventDefault();
		var undef;
		xmove = undef;
		document.onmousemove = null;
	}
	map.mapDiv.onmousedown = function(e){
		if( map.IE ) e = window.event;
		else e.preventDefault();
		document.onmouseup = disableDrag;
		document.onmousemove = moveMap;
	};
	
	// double click zoom:
	map.mapDiv.ondblclick = function(e){
		if( map.IE ) e = window.event;
		var el = map.mapDiv;
		var mapX = 0, mapY = 0;
		do {
			mapX += el.offsetLeft;
			mapY += el.offsetTop;
		} while( el = el.offsetParent );
		// alert( e.pageX );
		var x = (e.pageX||e.clientX)-mapX, y = (e.pageY||e.clientY)-mapY; // this will not work properly in IE if the page is scrolled!
		map.zoomIn( x, y );
	};

	// add zoom buttons
	var zoomInButton = document.createElement('div');
	// zoomInButton.setAttribute('class', 'mapiator_zoom_in');
	zoomInButton.id = 'mapiator_zoom_in';

	map.mapDiv.appendChild( zoomInButton );
	zoomInButton.onmouseup = function(){map.zoomIn();};

	var zoomOutButton = document.createElement('div');
	zoomOutButton.id = 'mapiator_zoom_out';

	map.mapDiv.appendChild( zoomOutButton );
	zoomOutButton.onmouseup = function(){map.zoomOut();};

	function preventDblClick(e){
		if( map.IE ) window.event.cancelBubble=true
		else e.stopPropagation();
	};
	zoomInButton.ondblclick = preventDblClick;
	zoomOutButton.ondblclick = preventDblClick;

	setTimeout( function() {
		util.addCssRule( '#mapiator_zoom_in, #mapiator_zoom_out', 'position:absolute; z-index:30; width:48px; height:37px; left:15px;' );
		util.addCssRule( '#mapiator_zoom_in', 'background:url(../images/zoomIn_blur.png); top:15px;' );
		util.addCssRule( '#mapiator_zoom_out', 'background:url(../images/zoomOut_blur.png); top:55px;' );
	}, 1);
};

Mapiator.iPhoneController = function(map) {
	var currentX, currentY;
	var mapDiv = map.mapDiv;
    mapDiv.addEventListener( 'touchstart', function(e){
        if(e.touches.length == 1){ // Only deal with one finger
            var touch = e.touches[0]; // Get the information for finger #1
            currentX = touch.pageX;
            currentY = touch.pageY;
        } 
    });

    mapDiv.addEventListener( 'touchmove', function(e){
        e.preventDefault();
        if(e.touches.length == 1){
            var touch = e.touches[0];
            diffX = touch.pageX - currentX;
            diffY = touch.pageY - currentY;

            map.moveByPx(diffX,diffY);

            currentX = touch.pageX;
            currentY = touch.pageY;
        }
    });

	// zoom:
    mapDiv.addEventListener( 'gestureend', function(e){
        // note: this does not work if the default is prevented!
        if( e.scale > 1) map.zoomIn();
        if( e.scale < 1) map.zoomOut();
    });
	
}