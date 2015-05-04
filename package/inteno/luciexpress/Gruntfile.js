module.exports = function(grunt){
	var glob = require("glob"); 
	var async = require("async"); 
	var fs = require("fs"); 
	
	grunt.loadNpmTasks('grunt-angular-gettext'); 
	grunt.initConfig({
		nggettext_extract: {
			pot: {
				files: {
					'po/template.pot': [
						'js/*.js',
						'htdocs/plugins/**/*.html', 
						'htdocs/plugins/**/*.js', 
						'htdocs/themes/**/*.html', 
						'htdocs/themes/**/*.js']
				}
			}
		}, 
		nggettext_compile: {
			all: {
				files: {
					'htdocs/lib/js/translations.js': ['po/*.po']
				}
			}
		}, 
		extract_titles: {
			options: {
				files: {
					'po/titles.pot': [
						"share/menu.d/*.json"
					]
				}
			}
		}
	}); 
	grunt.registerTask("extract_titles", 'Extracts titles from menu files', function(arg){
		console.log(JSON.stringify(this.options())+" "+arg);
		var opts = this.options(); 
		var done = this.async(); 
		if(opts.files){
			async.eachSeries(Object.keys(opts.files), function(file, next){
				var output = []; 
				async.eachSeries(opts.files[file], function(pattern, next){
					glob(pattern, null, function(err, files){
						console.log(JSON.stringify(files)); 
						files.map(function(file){
							var obj = JSON.parse(fs.readFileSync(file)); 
							output.push("# "+file); 
							Object.keys(obj).map(function(k){
								output.push("msgid \""+k.replace(/\//g, ".")+".title\""); 
								output.push("msgstr \"\""); 
							});  
						}); 
						next(); 
					}); 
				}, function(err){
					//console.log("Writing file: "+file+" "+output.join("\n")); 
					fs.writeFileSync(file, output.join("\n")); 
					next(); 
				}); 
			}, function(){
				done(); 
			}); 
		}
		// combine all menu files we have locally
		/*fs.readdir("share/menu.d", function(err, files){
			files.map(function(file){
				var obj = JSON.parse(fs.readFileSync("share/menu.d/"+file)); 
				Object.keys(obj).map(function(k){
					menu[k] = obj[k]; 
				});  
			}); 
			next({
				menu: menu
			}); 
		}); */
	}); 
	grunt.registerTask("compile_pot", "Compiles all pot files into one 'all.pot'", function(){
		var exec = require('child_process').exec;
		var done = this.async(); 
		exec("rm po/all.pot ; msgcat po/*.pot > po/all.pot", function(){
			done(); 
		}); 
	}); 
	grunt.registerTask('default', ['nggettext_extract', 'nggettext_compile', "extract_titles", "compile_pot"]);
	
}
