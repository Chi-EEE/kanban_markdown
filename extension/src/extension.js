// The module 'vscode' contains the VS Code extensibility API

const { KanbanMarkdownEditorProvider } = require('./lib/kanban_markdown_editor');

// Import the module and reference it with the alias vscode in your code below
const vscode = require('vscode');

// This method is called when your extension is activated
// Your extension is activated the very first time the command is executed

/**
 * @param {vscode.ExtensionContext} context
 */
function activate(context) {
	context.subscriptions.push(KanbanMarkdownEditorProvider.register(context));
}

// This method is called when your extension is deactivated
function deactivate() { }

module.exports = {
	activate,
	deactivate
}
