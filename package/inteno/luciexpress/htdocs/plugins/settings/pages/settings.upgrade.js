;(jQuery && jQuery.fn.upload) || (function( $) {
		// abort if xhr progress is not supported
	if( !($.support.ajaxProgress = ("onprogress" in $.ajaxSettings.xhr()))) {
		return;
	}

	var _ajax = $.ajax;
	$.ajax = function ajax( url, options) {
			// If url is an object, simulate pre-1.5 signature
		if ( typeof( url) === "object" ) {
			options = url;
			url = options.url;
		}

			// Force options to be an object
		options = options || {};

		var deferred = $.Deferred();
		var _xhr = options.xhr || $.ajaxSettings.xhr;
		var jqXHR;
		options.xhr = function() {
				// Reference to the extended options object
			var options = this;
			var xhr = _xhr.call( $.ajaxSettings);
			if( xhr) {
				var progressListener = function( /*true | false*/upload) {
					return function( event) {
						/*
						 * trigger the global event.
						 * function handler( jqEvent, progressEvent, upload, jqXHR) {}
						 */
						options.global && $.event.trigger( "ajaxProgress", [ event, upload, jqXHR]);

							/*
							 * trigger the local event.
							 * function handler(jqXHR, progressEvent, upload)
							 */
						$.isFunction( options.progress) && options.progress( jqXHR, event, upload);

						deferred.notifyWith( jqXHR, [event, upload]);
					};
				};

				xhr.upload.addEventListener( "progress", progressListener( true), false);
				xhr.addEventListener( "progress", progressListener( false), false);
			}
			return xhr;
		};

		jqXHR = _ajax.call( this, url, options);

			// delegate all jqXHR promise methods to our deferred
		for( var method in deferred.promise()) {
			jqXHR[ method]( deferred[ method]);
		}
		jqXHR.progress = deferred.progress;

			// overwrite the jqXHR promise methods with our promise and return the patched jqXHR
		return jqXHR;
	};

		/**
		 * jQuery.upload( url [, data] [, success(data, textStatus, jqXHR)] [, dataType] )
		 *
		 * @param url
		 *         A string containing the URL to which the request is sent.
		 * @param data
		 *         A map or string that is sent to the server with the request.
		 * @param success(data, textStatus, jqXHR)
		 *         A callback function that is executed if the request succeeds.
		 * @param dataType
		 *         The type of data expected from the server. Default: Intelligent Guess (xml, json, script, text, html).
		 *
		 * This is a shorthand Ajax function, which is equivalent to:
		 * .ajax({
		 *		processData	: false,
		 *		contentType	: false,
		 *		type		: 'POST',
		 *		url			: url,
		 *		data		: data,
		 *		success		: callback,
		 *		dataType	: type
		 *	});
		 *
		 * @return jqXHR
		 */
	$.upload = function( url, data, callback, type) {
			// shift arguments if data argument was omitted
		if ( jQuery.isFunction( data ) ) {
			type = type || callback;
			callback = data;
			data = undefined;
		}

		return $.ajax({
			/*
			 * processData and contentType must be false to prevent jQuery
			 * setting its own defaults ... which would result in nonsense
			 */
			processData	: false,
			contentType	: false,
			type		: 'POST',
			url			: url,
			data		: data,
			success		: callback,
			dataType	: type
		});
	};
})( jQuery);

$juci.module("settings")
.controller("SettingsUpgradeCtrl", function($scope, $uci, $session){
	$scope.sessionID = $session.sid;
	$scope.uploadFilename = "/tmp/firmware.bin";
	$scope.usbFileName = "()"; 
	console.log("SID: "+$scope.sessionID);  
	
	$uci.sync("system").done(function(){
		if($uci.system.upgrade && $uci.system.upgrade.fw_upload_path.value){
			$scope.uploadFilename = $uci.system.upgrade.fw_upload_path.value; 
			console.log("Using upload path from config: "+$scope.uploadFilename); 
		}
	}); 
	
	$scope.onUploadComplete = function(result){
		console.log("Upload completed: "+JSON.stringify(result)); 
	}
	setInterval(function checkUpload(){
		var iframe = $("#postiframe"); 
		var json = iframe.contents().text();
		if(json.length) {
			$scope.onUploadComplete(JSON.parse(json)); 
			iframe.contents().html(""); 
		}
	}, 500); 
	$scope.onUploadUpgrade = function(){
		var formData = new FormData($('uploadForm')[0]); 
		
		/*$.upload( "/cgi-bin/luci-upload", new FormData($('*uploadForm*')[0]))
		.progress( function( progressEvent, upload) {
				if( progressEvent.lengthComputable) {
						var percent = Math.round( progressEvent.loaded * 100 / progressEvent.total) + '%';
						if( upload) {
								console.log( percent + ' uploaded');
						} else {
								console.log( percent + ' downloaded');
						}
				}
		})
		.done( function() {
				console.log( 'Finished upload');                    
		});*/
		$.ajax({
			type        : 'POST',
			url         : 'http://192.168.1.4/cgi-bin/luci-upload',
			data: formData,
			cache: false,
			contentType: false,
			processData: false,
			xhr: function() {  // custom xhr
					myXhr = $.ajaxSettings.xhr();
					if(myXhr.upload){ // if upload property exists
							myXhr.upload.addEventListener('progress', function(){
								console.log("Progress"); 
							}, false); // progressbar
					}
					return myXhr;
			},
			success     : function(data) {
				console.log("Upload completed!"); 
			}, 
			error: function(data){
				console.log("Error: "+JSON.stringify(data)); 
			}
		}, "json"); 
	}
}); 
