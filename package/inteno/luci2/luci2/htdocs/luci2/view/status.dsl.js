L.ui.view.extend({
	title: L.tr('ADSL Status'),
	description: L.tr('Router ADSL status.'),

	getDSLStatus: L.rpc.declare({
		object: 'luci2.network',
		method: 'dslstats',
		expect: { data: '' }
	}),

	execute: function() {
		return this.getDSLStatus().then(function(data){
			var dslstats = {}; 
			try { dslstats = JSON.parse(data); } catch(e){}
			
			// todo fields
			with({dslstats: dslstats.dslstats}){
				dslstats.ip = "TODO"; 
				dslstats.ipstate = "TODO"; 
				dslstats.mode_details = "TODO"; 
				dslstats.line_status_configured = "TODO"; 
				dslstats.line_type_configured = "TODO"; 
				dslstats.line_type = "TODO"; 
			}
			
			// compute floating point values (because ubus blobs do not support floats yet)
			function reconstruct_floats(obj) {
				for (var property in obj) {
					if (obj.hasOwnProperty(property)) {
						if (typeof obj[property] == "object") {
							reconstruct_floats(obj[property]);
						} else {
							var matches = property.match(/(.*)_x([\d]*)/); 
							if(matches && matches.length == 3){
								try {
									obj[matches[1]] = parseFloat(String(obj[property])) / parseFloat(matches[2]); 
								} catch(e) {
									obj[matches[1]] = "Err"; 
								}
							}
						}
					}
				}
			}
			reconstruct_floats(dslstats); 
			
			// translate labels
			$("*[translate]").each(function(){
				$(this).html(L.tr($(this).html())); 
			}); 
			
			// update the view
			$("span").each(function(){
				var model = $(this).attr("ng-model"); 
				try {
					if(model) { 
						with({dslstats: dslstats.dslstats}) { 
							$(this).html(eval(model)); 
						}
					}
				} catch(e) {}
			}); 
			
		}); 
	}
});
