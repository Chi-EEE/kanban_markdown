const { getNonce } = require('./util');
const zlib = require('zlib');

const vscode = require('vscode');
const { KanbanMarkdownServer } = require('./kanban_markdown_server');

/**
 * @implements {vscode.CustomTextEditorProvider}
 */
class KanbanMarkdownEditorProvider {

    /**
     * @public
     * @param {vscode.ExtensionContext} context 
     * @returns {vscode.Disposable}
     */
    static register(context) {
        const provider = new KanbanMarkdownEditorProvider(context);
        const providerRegistration = vscode.window.registerCustomEditorProvider(KanbanMarkdownEditorProvider.viewType, provider);
        return providerRegistration;
    }

    /**
     * @private
     * @readonly
     */
    static viewType = 'kanban-markdown.editor';

    /**
     * @param {vscode.ExtensionContext} context 
     */
    constructor(context) {
        this.context = context;
    }

    /**
     * @public
     * @param {vscode.TextDocument} document 
     * @param {vscode.WebviewPanel} webviewPanel 
     * @param {vscode.CancellationToken} token 
     */
    resolveCustomTextEditor(document, webviewPanel, token) {
        // Setup initial content for the webview
        // This cannot be done in the constructor because the webviewPanel is not available yet
        this.server = new KanbanMarkdownServer(this.context);

        console.log('Kanban Markdown Editor: ', document.uri.fsPath)

        this.server.sendRequest({
            type: 'parse',
            file: document.uri.fsPath,
        }).then(() => {
            console.log('File parsed');

            webviewPanel.webview.options = {
                enableScripts: true,
            };

            webviewPanel.webview.html = this.getHtmlForWebview(webviewPanel.webview);

            /**
             * @param {any} data
             */
            function updateWebview(data) {
                webviewPanel.webview.postMessage({
                    type: 'update',
                    text: data,
                });
            }

            const changeDocumentSubscription = vscode.workspace.onDidChangeTextDocument(e => {
                if (e.document.uri.toString() === document.uri.toString()) {
                    this.server.sendRequest({
                        type: 'get',
                        format: 'json',
                    }).then(data => {
                        updateWebview(data);
                    });
                }
            });

            // Make sure we get rid of the listener when our editor is closed.
            webviewPanel.onDidDispose(() => {
                this.server.close();
                changeDocumentSubscription.dispose();
            });

            webviewPanel.webview.onDidReceiveMessage(e => {
                switch (e.type) {
                    case 'update':
                    case 'create':
                    case 'delete':
                    case 'move':
                        this.sendCommand(document, e);
                        return;
                }
            });

            this.server.sendRequest({
                type: 'get',
                format: 'json',
            }).then(data => {
                updateWebview(data);
            });
        });
    }

    /**
     * @private
     * @param {vscode.Webview} webview 
     * @returns {string}
     */
    getHtmlForWebview(webview) {
        // Local path to script and css for the webview
        const scriptUri = webview.asWebviewUri(vscode.Uri.joinPath(
            this.context.extensionUri, 'out', 'compiled', 'bundled.js'));

        const styleMainUri = webview.asWebviewUri(vscode.Uri.joinPath(
            this.context.extensionUri, 'out', 'compiled', 'bundled.css'));

        // Use a nonce to whitelist which scripts can be run
        const nonce = getNonce();

        return /* html */`
        <!DOCTYPE html>
        <html lang="en">
            <head>
                <meta charset="UTF-8">
                <meta http-equiv="Content-Security-Policy" content="default-src 'none'; img-src ${webview.cspSource}; style-src ${webview.cspSource} 'unsafe-inline'; script-src 'nonce-${nonce}';">
                <meta name="viewport" content="width=device-width, initial-scale=1.0">
                <script type="module" nonce="${nonce}" crossorigin src="${scriptUri}"></script>
                <link href="${styleMainUri}" crossorigin rel="stylesheet" />
                <title>Kanban Markdown</title>
            </head>
            <body>
                <script nonce="${nonce}">
                    const vscode = acquireVsCodeApi();
                </script>
                <noscript>You need to enable JavaScript to run this app.</noscript>
                <div id="root"></div>
            </body>
        </html>
`;
    }

    /**
     * 
     * @param {vscode.TextDocument} document 
     * @param {*} e 
     * @returns 
     */
    sendCommand(document, e) {
        this.server.sendRequest({
            type: 'commands',
            commands: [
                {
                    action: e.type,
                    path: e.path,
                    value: e.value
                }
            ]
        }).then(() => {
            return this.server.sendRequest({
                type: 'get',
                format: 'markdown',
            });
        }).then(data => {
            this.decompressGzipString(data.markdown, (/** @type {any} */ err, /** @type {string} */ markdown) => {
                if (err) {
                    console.error('Error decompressing string:', err);
                    return;
                }
                this.updateTextDocument(document, markdown);
            });
        }).catch(error => {
            console.error("Error in processing requests:", error);
        });
    }

    /**
     * @private
     * @param {vscode.TextDocument} document 
     * @param {string} markdown
     * @returns 
     */
    updateTextDocument(document, markdown) {
        const edit = new vscode.WorkspaceEdit();

        // Just replace the entire document every time for this example extension.
        // A more complete extension should compute minimal edits instead.
        edit.replace(
            document.uri,
            new vscode.Range(0, 0, document.lineCount, 0),
            markdown);

        return vscode.workspace.applyEdit(edit);
    }

    // @ts-ignore
    decompressGzipString(gzipString, callback) {
        let buffer = Buffer.from(gzipString, 'base64');
        zlib.gunzip(buffer, (err, decompressedBuffer) => {
            if (err) {
                return callback(err);
            }
            let decompressedString = decompressedBuffer.toString('utf-8');
            callback(null, decompressedString);
        });
    }

}

module.exports = {
    KanbanMarkdownEditorProvider
}