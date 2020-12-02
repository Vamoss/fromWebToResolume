# From Web To Resolume
Get messages sent throw Microsoft Form via Sharepoint API, render this messages in a fashined style and send to Resolume via Syphon.

It consists with two apps:
  - A node application that get the messages from Sharepoint OAUTH.
  - A openframeworks application that get these messages, render and send to Resolume in Syphon protocol.
# Configuration
In [node_gateway](./node_gateway), create a copy of [.env.example](./node_gateway/.env.example) and name it `.env`.
Change the file variables with your Microsoft Sharepoint credentials. [More information of how to retrieve that](https://github.com/s-KaiNet/node-sp-auth/wiki/SharePoint%20Online%20addin%20only%20authentication).
# Create a standalone Node application
```sh
npm install -g pkg
pkg . --targets node12-mac
```
# OSs
It was only tested in Mac OS, but should work in other systems too.