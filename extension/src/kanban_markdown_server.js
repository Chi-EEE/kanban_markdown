const fs = require('fs');
const { spawn } = require('child_process');
const vscode = require('vscode');

function uuidv4() {
    return "10000000-1000-4000-8000-100000000000".replace(/[018]/g, c =>
        (+c ^ crypto.getRandomValues(new Uint8Array(1))[0] & 15 >> +c / 4).toString(16)
    );
}

class KanbanMarkdownServer {
    /**
     * @param {vscode.ExtensionContext} context 
     */
    constructor(context) {
        this.context = context;
        this.requestMap = new Map();

        // Check if server exists
        const server_path = vscode.Uri.joinPath(this.context.extensionUri, 'server', 'kanban-markdown_server.exe');
        if (!fs.existsSync(server_path.fsPath)) {
            console.log("Server not found");
            return;
        }

        const server = spawn(server_path.fsPath);

        this.server = server;

        this.server.stdout.on('data', (data) => {
            data = data.toString();
            for (let line of data.split('\n')) {
                if (line === '') {
                    continue;
                }
                try {
                    const response = JSON.parse(line);

                    if (this.requestMap.has(response.id)) {
                        this.requestMap.get(response.id)(response);
                        this.requestMap.delete(response.id);
                    } else {
                        console.error('Response not found:', response);
                    }
                } catch (error) {
                    console.error('Failed to process response:', error);
                    console.error('Response:', line);
                }
            }
        });
    }

    /**
     * 
     * @param {any} request 
     */
    sendRequest(request) {
        const id = uuidv4();
        request.id = id;
        console.log('Sending request:', JSON.stringify(request));
        return new Promise((resolve, reject) => {
            this.requestMap.set(id, resolve);

            let data = JSON.stringify(request);
            this.server.stdin.cork();
            this.server.stdin.write(data + '\n');
            this.server.stdin.uncork();
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
};
