const fs = require('fs');
const { spawn } = require('child_process');
const vscode = require('vscode');
const crypto = require('crypto');

function uuidv4() {
    return "10000000-1000-4000-8000-100000000000".replace(/[018]/g, c =>
        (+c ^ crypto.getRandomValues(new Uint8Array(1))[0] & 15 >> +c / 4).toString(16)
    );
}

const getHash = path => new Promise((resolve, reject) => {
    const hash = crypto.createHash('sha256');
    const rs = fs.createReadStream(path);
    rs.on('error', reject);
    rs.on('data', chunk => hash.update(chunk));
    rs.on('end', () => resolve(hash.digest('hex')));
})

class KanbanMarkdownServer {
    /**
     * @param {vscode.ExtensionContext} context 
     */
    constructor(context) {
        this.context = context;
        this.requestMap = new Map();
    }

    static async new(context) {
        const server = new KanbanMarkdownServer(context);
        await server.initializeServer();
        return server;
    }

    async initializeServer() {
        // Check if server exists
        const server_path = vscode.Uri.joinPath(this.context.extensionUri, 'server', 'kanban-markdown_server.exe');
        if (!fs.existsSync(server_path.fsPath)) {
            console.log("Server not found");
            return;
        }

        const server_hash_path = vscode.Uri.joinPath(this.context.extensionUri, 'server', 'kanban-markdown_server.exe.sha256');
        if (!fs.existsSync(server_hash_path.fsPath)) {
            console.log("Server hash not found");
            return;
        }

        try {
            const hashValue = await getHash(server_path.fsPath);
            const correctHash = fs.readFileSync(server_hash_path.fsPath, 'utf-8').trim();
            if (hashValue !== correctHash) {
                console.error(`Failed to verify hash: ${hashValue}`);
                return;
            }
            console.log(`Successfully verified hash: ${hashValue}`);
        } catch (error) {
            console.error(`Failed to verify hash: ${error}`);
            return;
        }

        this.server = spawn(server_path.fsPath);

        this.server.stdout.on('data', (data) => {
            data = data.toString();
            for (let line of data.split('\n')) {
                if (line === '') {
                    continue;
                }
                console.log('Received data:', line);
                try {
                    const response = JSON.parse(line);

                    if (this.requestMap.has(response.id)) {
                        // console.log('Received response:', JSON.stringify(response));
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

        this.server.on('error', (error) => {
            console.error("Unexpected error:", error);
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
