#pragma once

#define WEBVIEW_WINAPI

#include<iostream>
#include<fstream>
#include<string>
#include<cstdlib>
#include<windows.h>
#include<winsock.h>
#include <ole2.h>
#include <atldbcli.h>
#include<chrono>
#include"stb\stb_image.h"
#include "json.hpp"
#include "webview.h"

using json=nlohmann::json; 
using namespace std;

class Logger {
	private: 
		std::fstream infile;
		int init=0;
	
	public: 
	
		Logger() {
			
			infile.open("logi.txt", std::ios::in | std::ios::out | std::ios::app);
		}
		
		void log(std::string in);
	
		void exit() {infile.close();}
	
} logi;

/*
class Utilities {
	
	public: 
		int randnum() {
			srand(std::chrono::system_clock::now().time_since_epoch().count());
		
			return rand();
		}
} util;
*/

class MediaManager {
	
	public: 
		void MediaDiscovery(double minMP) {

			HRESULT hr = CoInitialize(NULL);
			if (FAILED(hr)) return;

			// We use the "Program ID" to find the Search Provider without ATL
			CLSID clsidConnection;
			CLSIDFromProgID(L"ADODB.Connection", &clsidConnection);

			// Create the connection object
			IUnknown* pUnk = nullptr;
			hr = CoCreateInstance(clsidConnection, NULL, CLSCTX_INPROC_SERVER, IID_IUnknown, (void**)&pUnk);
			
			// 1. Define the SQL-like query
			// We use System.Image.HorizontalSize * System.Image.VerticalSize
			std::wstring query = L"SELECT System.ItemPathDisplay, System.Image.HorizontalSize, System.Image.VerticalSize "
						 L"FROM SystemIndex "
						 L"WHERE System.Kind = 'picture' "
						 L"AND (System.FileExtension = '.jpg' OR System.FileExtension = '.jpeg' OR System.FileExtension = '.png') "
						 L"AND System.Image.HorizontalSize * System.Image.VerticalSize >= ";
			query += std::to_wstring(MPtoPixels(minMP));

			// 2. Set up the OLE DB connection to the Search Indexer
			CDataSource ds;
			CSession ss;
			CCommand<CDynamicAccessor> cmd;

			// Connection string for the Windows Search provider
			hr = ds.OpenFromInitializationString(L"provider=Search.CollatorDSO.1;EXTENDED PROPERTIES=\"Application=Windows\"");
			
			if (SUCCEEDED(hr)) hr = ss.Open(ds);
			if (SUCCEEDED(hr)) {
				hr = cmd.Open(ss, query.c_str());
				if (SUCCEEDED(hr)) {
					
					// 3. Iterate through results
					int count=0;
					while (cmd.MoveNext() == S_OK) {
						wchar_t* path = (wchar_t*)cmd.GetValue(1);
						UINT32 width = *(UINT32*)cmd.GetValue(2);
						UINT32 height = *(UINT32*)cmd.GetValue(3);

						double actualMP = (double)(width * height) / 1000000.0;
						
						
						fnames[count]=path;
						count++;
					}
				}
			}

			cmd.Close();
			ss.Close();
			ds.Close();
			CoUninitialize();
		}
		
		void cleanup() {}
} media;

/*
class Config {
	
	private: 
		int count;
		fstream mainfile; 
	
	public: 
				
		int readconfig(std::string file) {
			
			//open file 
			mainfile.open(file, std::ios::in | std::ios::out | std::ios::app);
			
			//read settings from file
			if(xmlDocPtr doc=xmlreadfile(file, NULL, 0)) {
				
				logi.log("Problem opening config file. Exiting program");
				exit -1;
			};
			
			xmlNodePtr root=xmlDocGetRootElement(doc);
			
			// assign them to specific internal settings
			for(xmlNodePtr cur= root->children; cur!=NULL; c=c->next) {
				if(cur->type == XML_ELEMENT_NODE) {
					
					// read the 'name' attribute of the tag
					xmlChar* content= xmlNodeGetContent(cur);

					// assign it to its innertext
					wall.settings[id]=content;
				}
			}
			
			xmlFreeDoc(doc);
			xmlCleanupParser();
			
			//close file
		}
		
		void writeconfig(string in) {
			
			//build the json
			
			//append to file
		}
		
		void cleanup() {mainfile.close();}
		
}, config

class Window {
	
	private: 
		webview:webview mw(true, nullptr);
		WNDCLASS wc;
		
		void updateui(string control, string value) {
			
			json msg;
			msg["id"]=control;
			msg["value"]=value;
			
			// Send it to front end
			std::string js="window.dispatchEvent(new CustomEvent('front-update, {detail: "+msg.dump() + "}));";
			js.eval();
		}
		
		string parseinput(string in) {
				// 1. Parse the JSON coming from the frontend
				auto data = json::parse(in);
				
				std::string cmd = data["command"];
				
				// 2. Route the command to your native Windows logic
				if (cmd == "ok") {
					ok();
				} 
				else {
					
				}

				return "{\"status\": \"ok\"}"; // Send a response back to JS
				
		}
			
	public: 
		int width=1280, height=1280;
		HWND id=NULL;
		
		int init() {
			
			logi.log("Creating Main Window");
			
			// Create dummy window class
			wc.lpfnWndProc = WndProc;
			wc.lpszClassName = L"ProGambler";
			RegisterClass(&wc);
			
			mw.settitle("ProGambler");
			mw.set_size(1280, 720, WEBVIEW_HINT_NONE);
			
			// initialize communication
			mw.bind("posttofront", [this](string msg) -> string{
				
				return this->parseinput(msg);
			});

			mw.navigate("file:///main.html");
			mw.run();
	
			logi.log("Done creating window");
			return 0;
		}
			
		void out(std::string in) {
			
			updateui("main", in);
		}
		
		void ok();
		
		int removechild() {}

} mwin;

*/

void Logger::log(std::string in) {			

	if(init) {init=1;}
	
	// build the input string
	string ft = ">> " + in + "\r\n";
	
	// send to output sources
	// mwin.out(ft);
	cout << ft; 
	infile << ft;
	
} logi;

class Gambler {
	
	private: 
		string fnames[60000];
		std::map<std::string, std::string> settings;
		int count; 
		double minres=750000;
		
		void flashylights() {}
		
	public: 
		string currentimg;
		
		void init() {
			
			// mwin.init();
			// config.readconfig("settings.json");
			media.MediaDiscovery(minres);
			
			logi.log("successfully tested backend");
			
		}
		
		string gamble() {
			
			// generate random number
			//double random = util.randnum()%60000;
			
			// change the video layers
			flashylights();
			
			// draw the active layers
			currentimg=fnames[rand()];
			return currentimg;
		}
		
		void cleanup() {
			
			// config.cleanup();
			// logi.cleanup();
			media.cleanup();
			
		}
		
} gambler;

/*
void Window::ok() {
			
			// Gamble
			
			//update ui
			out(gambler.gamble());
}
*/

void MediaManager::MediaDiscovery(double minMP) {

	HRESULT hr = CoInitialize(NULL);
	if (FAILED(hr)) return;

	// 1. Define the SQL-like query
	// We use System.Image.HorizontalSize * System.Image.VerticalSize
	std::wstring query = L"SELECT System.ItemPathDisplay, System.Image.HorizontalSize, System.Image.VerticalSize "
				 L"FROM SystemIndex "
				 L"WHERE System.Kind = 'picture' "
				 L"AND (System.FileExtension = '.jpg' OR System.FileExtension = '.jpeg' OR System.FileExtension = '.png') "
				 L"AND System.Image.HorizontalSize * System.Image.VerticalSize >= ";
	query += std::to_wstring(MPtoPixels(minMP));

	// 2. Set up the OLE DB connection to the Search Indexer
	CDataSource ds;
	CSession ss;
	CCommand<CDynamicAccessor> cmd;

	// Connection string for the Windows Search provider
	hr = ds.OpenFromInitializationString(L"provider=Search.CollatorDSO.1;EXTENDED PROPERTIES=\"Application=Windows\"");

	if (SUCCEEDED(hr)) hr = ss.Open(ds);
	if (SUCCEEDED(hr)) {
		hr = cmd.Open(ss, query.c_str());
		if (SUCCEEDED(hr)) {
			
			// 3. Iterate through results
			int count=0;
			while (cmd.MoveNext() == S_OK) {
				wchar_t* path = (wchar_t*)cmd.GetValue(1);
				UINT32 width = *(UINT32*)cmd.GetValue(2);
				UINT32 height = *(UINT32*)cmd.GetValue(3);

				double actualMP = (double)(width * height) / 1000000.0;
				
				
				fnames[count]=path;
				count++;
			}
		}
	}

	cmd.Close();
	ss.Close();
	ds.Close();
	CoUninitialize();
}