const result = require('dotenv').config()

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
	headers['Accept'] = 'application/json;odata=verbose';
	console.log("Sharepoint credentials ready!");
})
.catch(error => {
	console.error("Sharepoint credentials error!");
	console.error(error);
});

//HTTP SERVER
const http = require('http');

const requestListener = function (req, res) {
	if(req.url != "/"){
		console.error("not found", req.url);
		res.writeHead(404);
		res.end();
		return;
	}
	console.log("new request", req.url);
    res.setHeader('Content-Type', 'application/json');
	res.writeHead(200);
	request.get({
		url: process.env.URL_DATA,
		headers: headers
	}).then(response => {
		var result = [];
		var obj = JSON.parse(response).d;
		obj.results.forEach((m, i)=>{
			//ignore list titles
			if(i%2==0) return;
			result.push({
				id: m.ID,
				messagem: m.Title,
				createdAt: m.Created,
				updatedAt: m.Modified
			})
		})
		res.end(JSON.stringify(result));
	}).catch(error => {
		console.error("Sharepoint get error!");
		console.error(error);
		res.end(JSON.stringify([]));
	});
}

const server = http.createServer(requestListener);
server.listen(8080);

console.log("App listening at port 8080");