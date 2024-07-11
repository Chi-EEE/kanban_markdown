const fs = require('fs');
const spawn = require('child_process').spawn;

const vscode = require('vscode');

class KanbanMarkdownServer {
    /**
     * @param {vscode.ExtensionContext} context 
     */
    constructor(context) {
        this.context = context;
        // Check if server exists
        const server_path = vscode.Uri.joinPath(this.context.extensionUri, 'server', 'kanban-markdown_server.exe');
        if (!fs.existsSync(server_path.fsPath)) {
            console.log("Server not found");
            return;
        }

        const server = spawn(server_path.fsPath);

        this.server = server;
    }

    /**
     * 
     * @param {*} request 
     */
    sendRequest(request) {
        this.server.stdin.cork();
        this.server.stdin.write(request.toString() + '\n');
        this.server.stdin.uncork();
    }

    /**
     * Message Callback.
     *
     * @callback MessageCallback
     * @param {string} data
     */

    /**
     * 
     * @param {MessageCallback} callback 
     */
    onMessage(callback) {
        this.server.stdout.on('data', function (data) {
            callback(data.toString());
        });
    }

    /**
     * 
     */
    close() {
        this.server.kill('SIGKILL');
    }
}

module.exports = {
    KanbanMarkdownServer
}