export namespace KanbanMarkdown {
    export interface State {
        selectedList: KanbanList | undefined;
        selectedTask: KanbanTask | undefined;
        kanban_board: KanbanBoard;
    }

    export interface KanbanBoard {
        name: string;
        properties: {
            color: string;
            created: number;
            last_modified: number;
            version: number;
            checksum: string;
        };
        description: string;
        labels: KanbanLabel[];
        lists: KanbanList[];
    }

    export interface KanbanLabel {
        color: string;
        name: string;
        tasks: KanbanTask[];
    }

    export interface KanbanList {
        name: string;
        checked: boolean;
        tasks: KanbanTask[];
    }

    export interface KanbanTask {
        name: string;
        counter: number;
        checked: boolean;
        description: string;
        labels: KanbanLabel[];
        attachments: any[];
        checklist?: any[];
    }
}