/*
 * juci - javascript universal client interface
 *
 * Project Author: Martin K. Schröder <mkschreder.uk@gmail.com>
 * 
 * Copyright (C) 2012-2013 Inteno Broadband Technology AB. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */
 
JUCI.app.config(function($stateProvider) {
	var plugin_root = $juci.module("phone").plugin_root; 
	$stateProvider.state("phone", {
		url: "/phone", 
		onEnter: function($state){
			$juci.redirect("phone-call-log"); 
		},
	}); 
	
	// add the uci classes for voice client
	UCI.$registerConfig("voice_client"); 
	UCI.voice_client.$registerSectionType("brcm_line", {
		"extension": 			{ dvalue: '', type: String }, 
		"sip_account": 		{ dvalue: '', type: String }, 
		"noise":					{ dvalue: false, type: Boolean }, 
		"vad":						{ dvalue: false, type: Boolean }, 
		"txgain":					{ dvalue: false, type: Boolean }, 
		"rxgain":					{ dvalue: false, type: Boolean }, 
		"echo_cancel":		{ dvalue: true, type: Boolean }, 
		"callwaiting":		{ dvalue: false, type: Boolean }, 
		"clir":						{ dvalue: false, type: Boolean }, 
		"name":						{ dvalue: '', type: String }, 
		"instance":				{ dvalue: '', type: String }
	}); 
	UCI.voice_client.$registerSectionType("sip_service_provider",  {
		"name":							{ dvalue: "", type: String },
		"codec0":						{ dvalue: "alaw", type: String },  
		"codec1":						{ dvalue: "ulaw", type: String },       
		"codec2":						{ dvalue: "g729", type: String },   
		"codec3":						{ dvalue: "g726", type: String },
		"autoframing":			{ dvalue: false, type: Boolean },
		"cfim_on":					{ dvalue: "*21*", type: String },  
		"cfim_off":					{ dvalue: "#21#", type: String },
		"cfbs_on":					{ dvalue: "*61*", type: String }, 
		"cfbs_off":					{ dvalue: "#61#", type: String }, 
		"call_return":			{ dvalue: "*69", type: String },         
		"redial":						{ dvalue: "*66", type: String },   
		"is_fax":						{ dvalue: false, type: Boolean },      
		"transport":				{ dvalue: "udp", type: String },
		"priority_ulaw":		{ dvalue: 0, type: Number },
		"priority_alaw":		{ dvalue: 0, type: Number }, 
		"priority_g729":		{ dvalue: 0, type: Number },  
		"priority_g723":		{ dvalue: 0, type: Number },
		"priority_g726":		{ dvalue: 0, type: Number },   
		"enabled":					{ dvalue: true, type: Boolean },
		"target":						{ dvalue: "direct", type: String },       
		"call_lines":				{ dvalue: "BRCM/4", type: String },
		"mailbox":					{ dvalue: "", type: String },     
		"call_filter":			{ dvalue: "", type: String },     
		"domain":						{ dvalue: "", type: String },      
		"user":							{ dvalue: "", type: String }, 
		"authuser":					{ dvalue: "", type: String }, 
		"displayname":			{ dvalue: "", type: String }, 
		"ptime_ulaw":				{ dvalue: 20, type: Number }, 
		"ptime_g726":				{ dvalue: 20, type: Number },     
		"ptime_g729":				{ dvalue: 20, type: Number }, 
		"ptime_alaw":				{ dvalue: 20, type: Number }, 
		"host":							{ dvalue: "", type: String },  
		"outboundproxy":		{ dvalue: "", type: String }
	}); 
	UCI.voice_client.$registerSectionType("call_filter", { 
		"name":							{ dvalue: "", type: String }, 
		"block_outgoing":		{ dvalue: true, type: Boolean }, 
		"block_incoming":		{ dvalue: true, type: Boolean }, 
		"block_foreign": 		{ dvalue: true, type: Boolean }, // outgoing foreign
		"block_special_rate": { dvalue: false, type: Boolean } // outgoing special rate
	}); 
	UCI.voice_client.$registerSectionType("call_filter_rule_outgoing", {
		"owner": 						{ dvalue: "", type: String }, 
		"enabled": 					{ dvalue: true, type: Boolean },
		"extension": 				{ dvalue: "", type: String }
	}); 
	UCI.voice_client.$registerSectionType("call_filter_rule_incoming", {
		"owner": 						{ dvalue: "", type: String }, 
		"enabled": 					{ dvalue: true, type: Boolean },
		"extension": 				{ dvalue: "", type: String }
	}); 
}); 
