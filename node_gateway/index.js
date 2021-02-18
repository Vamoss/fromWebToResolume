//CONFIG
const path = require('path');
const dotenvAbsolutePath = path.join(__dirname, '.env');
const dotenv = require('dotenv').config({
	path: dotenvAbsolutePath
});
if (dotenv.error) {
	throw dotenv.error;
}
console.log(dotenv.parsed)

//Microsoft OAUTH
const spauth = require('node-sp-auth');
const request = require('request-promise');

console.log("Creating sharepoint credentials");
let headers;
spauth.getAuth(process.env.URL_CREDENTIALS, {
    clientId: process.env.CLIENT_ID,
    clientSecret: process.env.CLIENT_SECRET
})
.then(options => {
	headers = options.headers;
	headers['Accept'] = 'application/json;odata=nometadata';
	console.log("Sharepoint credentials ready!");
})
.catch(error => {
	console.error("Sharepoint credentials error!");
	console.error(error);
});

//HTTP SERVER
const http = require('http');

const requestListener = async function (req, res) {
	if(req.url != "/"){
		console.error("not found", req.url);
		res.writeHead(404);
		res.end();
		return;
	}
	if(!headers){
		console.error("headers not ready yet");
		res.writeHead(500);
		res.end();
		return;
	}
	console.log("new request", req.url);
	var startTime = new Date().getTime();
    res.setHeader('Content-Type', 'application/json');
	res.writeHead(200);
	var result = await loadUrl(process.env.URL_DATA, []);
	res.end(JSON.stringify(result));
	console.log("loaded", result.length, "in", new Date().getTime()-startTime, "ms");
}

const loadUrl = async function (url, arr) {
	console.log("Loading", url);
	return request.get({
		url: url,
		headers: headers
	}).then(async response => {
		var obj = JSON.parse(response);
		obj.value.forEach((m, i)=>{

			let message = m.Title;
			
			if(message == null) return;

			message = truncateOnWord(message, 150);

			arr.push({
				id: m.ID,
				mensagem: message,
				createdAt: m.Created,
				updatedAt: m.Modified
			})

		})
		if(obj["odata.nextLink"] && obj["odata.nextLink"] != url){
			return await loadUrl(obj["odata.nextLink"], arr);
		}else{
			return arr;
		}
	}).catch(error => {
		console.error("Sharepoint get error!");
		console.error(error);
		return arr;
	});
}

function truncateOnWord(str, limit) {
	if(str.length > limit){
		var trimmable = '\u0009\u000A\u000B\u000C\u000D\u0020\u00A0\u1680\u180E\u2000\u2001\u2002\u2003\u2004\u2005\u2006\u2007\u2008\u2009\u200A\u202F\u205F\u2028\u2029\u3000\uFEFF';
		var reg = new RegExp('(?=[' + trimmable + '])');
		var words = str.split(reg);
		var count = 0;
		return words.filter(function(word) {
			count += word.length;
			return count <= limit;
		}).join('') + '...';
	}else{
		return str;
	}
}

const server = http.createServer(requestListener);
server.listen(8080);

console.log("App listening at port 8080");