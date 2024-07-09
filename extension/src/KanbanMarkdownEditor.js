const { getNonce } = require('./util');

const vscode = require('vscode');

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
        webviewPanel.webview.options = {
            enableScripts: true,
        };

        webviewPanel.webview.html = this.getHtmlForWebview(webviewPanel.webview);

        function updateWebview() {
            webviewPanel.webview.postMessage({
                type: 'update',
                text: document.getText(),
            });
        }

        const changeDocumentSubscription = vscode.workspace.onDidChangeTextDocument(e => {
            if (e.document.uri.toString() === document.uri.toString()) {
                updateWebview();
            }
        });

        // Make sure we get rid of the listener when our editor is closed.
        webviewPanel.onDidDispose(() => {
            changeDocumentSubscription.dispose();
        });

        updateWebview();
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

        const styleResetUri = webview.asWebviewUri(vscode.Uri.joinPath(
            this.context.extensionUri, 'media', 'reset.css'));

        const styleVSCodeUri = webview.asWebviewUri(vscode.Uri.joinPath(
            this.context.extensionUri, 'media', 'vscode.css'));

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
    
                <link href="${styleResetUri}" rel="stylesheet" />
                <link href="${styleVSCodeUri}" rel="stylesheet" />
                <link href="${styleMainUri}" rel="stylesheet" />
    
                <title>Kanban Board</title>
            </head>
            <body>
                <div id="board">
                    <div class="list">
                        <input class="list-title" placeholder="Enter list title" />
                        <div class="cards">
                            <div class="card">
                                <input class="card-title" placeholder="Enter card title" />
                            </div>
                        </div>
                        <button class="add-card">Add another card</button>
                    </div>
                    <button id="add-list">Add another list</button>
                </div>
                
                <script nonce="${nonce}" src="${scriptUri}"></script>
            </body>
            </html>`;
    }

}

module.exports = {
    KanbanMarkdownEditorProvider
}