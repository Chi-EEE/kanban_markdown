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
        this.server = new KanbanMarkdownServer(this.context);

        console.log('Kanban Markdown Editor: ', document.uri.fsPath)

        this.server.sendRequest({
            type: 'parse',
            file: document.uri.fsPath,
        }).then(() => {
            webviewPanel.webview.options = {
                enableScripts: true,
            };

            webviewPanel.webview.html = this.getHtmlForWebview(webviewPanel.webview);

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
            this.context.extensionUri, 'media', 'kanban.js'));

        const styleMainUri = webview.asWebviewUri(vscode.Uri.joinPath(
            this.context.extensionUri, 'media', 'kanban.css'));

        // Use a nonce to whitelist which scripts can be run
        const nonce = getNonce();

        return /* html */`
            <!DOCTYPE html>
            <html lang="en">
            <head>
                <meta charset="UTF-8">
    
                <!--
                Use a content security policy to only allow loading images from https or from our extension directory,
                and only allow scripts that have a specific nonce.
                -->
                <meta http-equiv="Content-Security-Policy" content="default-src 'none'; img-src ${webview.cspSource}; style-src ${webview.cspSource}; script-src 'nonce-${nonce}';">
    
                <meta name="viewport" content="width=device-width, initial-scale=1.0">
    
                <link href="${styleMainUri}" rel="stylesheet" />

                <title>Kanban Board</title>
            </head>
            <body>
                <div id="title-bar">
                    <div id="editable-title">
                        <h1 id="kanban-title">Untitled Board</h1>
                        <input type="text" id="edit-title-input" />
                    </div>
                    <input type="color" id="background-color-picker" value="#A0A0A0">
                </div>

                <div id="board">
                    <button id="add-list">Add another list +</button>
                </div>
                
                <div id="card-modal" class="modal">
                    <div class="modal-content">
                        <span class="close">&times;</span>
                        <div class="modal-main">
                            <div class="modal-header">
                                <h2>Edit Card</h2>
                                <div class="label-bar">
                                    <button class="label-button">Label 1</button>
                                    <button class="label-button">Label 2</button>
                                    <button class="label-button">Label 3</button>
                                </div>
                            </div>
                            <div class="modal-body">
                                <label for="edit-card-title">Title</label>
                                <input type="text" id="edit-card-title" placeholder="Enter card title" />
                                <label for="edit-card-description">Description</label>
                                <textarea id="edit-card-description" placeholder="Enter card description"></textarea>
                            </div>
                            <div class="modal-footer">
                                <button id="save-card">Save</button>
                            </div>
                        </div>
                        <div class="modal-sidebar">
                            <button id="attachment-button">Attachments</button>
                        </div>
                    </div>
                </div>


                <script src="https://code.jquery.com/jquery-3.7.1.min.js" crossorigin="anonymous" nonce="${nonce}"></script>
                <script src="https://code.jquery.com/ui/1.13.3/jquery-ui.min.js" defer crossorigin="anonymous" nonce="${nonce}"></script>

                <script nonce="${nonce}" src="${scriptUri}"></script>
                </body>
            </html>`;
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
            this.decompressGzipString(data.markdown, (err, markdown) => {
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