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
        KanbanMarkdownServer.new(this.context).then(server => {
            console.log('Kanban Markdown Editor: ', document.uri.fsPath)

            server.sendRequest({
                type: 'parseFile',
                file: document.uri.fsPath,
            }).then(() => {
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
                        text: JSON.stringify(data),
                    });
                }

                const changeDocumentSubscription = vscode.workspace.onDidChangeTextDocument(e => {
                    if (e.document.uri.toString() === document.uri.toString()) {
                        this.compressGzipString(e.document.getText(), (/** @type {any} */ err, /** @type {string} */ compressedString) => {
                            server.sendRequest({
                                type: 'parseFileWithContent',
                                file: document.uri.fsPath,
                                content: compressedString,
                            }).then(server.sendRequest({
                                type: 'get',
                                format: 'json',
                            }).then(data => {
                                updateWebview(data);
                            }));
                        });
                    }
                });

                // Make sure we get rid of the listener when our editor is closed.
                webviewPanel.onDidDispose(() => {
                    server.close();
                    changeDocumentSubscription.dispose();
                });

                webviewPanel.webview.onDidReceiveMessage(e => {
                    this.sendCommands(server, document, e);
                });

                server.sendRequest({
                    type: 'get',
                    format: 'json',
                }).then(data => {
                    updateWebview(data);
                });
            });
        }).catch(error => {

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
            this.context.extensionUri, 'dist', 'frontend', 'bundled.js'));

        const styleMainUri = webview.asWebviewUri(vscode.Uri.joinPath(
            this.context.extensionUri, 'dist', 'frontend', 'bundled.css'));

        // Use a nonce to whitelist which scripts can be run
        const nonce = getNonce();

        // To allow SolidJS: https://github.com/withastro/astro/pull/2359
        return /* html */`
        <!DOCTYPE html>
        <html lang="en">
            <head>
                <meta charset="UTF-8">
                <meta http-equiv="Content-Security-Policy" content="default-src 'none'; img-src ${webview.cspSource}; style-src ${webview.cspSource} 'unsafe-inline'; script-src 'nonce-${nonce}';">
                <meta name="viewport" content="width=device-width, initial-scale=1.0">
                <script nonce="${nonce}">window._$HY||(_$HY={events:[],completed:new WeakSet,r:{}})</script>
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
     * @param {KanbanMarkdownServer} server
     * @param {vscode.TextDocument} document 
     * @param {*} e 
     * @returns 
     */
    sendCommands(server, document, e) {
        server.sendRequest({
            type: 'commands',
            commands: e.commands,
        }).then(() => {
            return server.sendRequest({
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

    compressGzipString(inputString, callback) {
        let buffer = Buffer.from(inputString, 'utf-8');
        zlib.gzip(buffer, (err, compressedBuffer) => {
            if (err) {
                return callback(err);
            }
            let compressedString = compressedBuffer.toString('base64');
            callback(null, compressedString);
        });
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