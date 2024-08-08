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

        list_name_tracker_map: Map<string, DuplicateNameTracker>;
        task_name_tracker_map: Map<string, DuplicateNameTracker>;

        labels: KanbanLabel[];
        lists: KanbanList[];
    }

    export class DuplicateNameTracker {
        private counter: number;
        private used_hash: Set<number>;

        constructor() {
            this.counter = 0;
            this.used_hash = new Set<number>();
        }

        getHash(): number {
            let counter: number;
            do {
                counter = ++this.counter;
            } while (this.used_hash.has(counter));
            this.used_hash.add(counter);
            return counter;
        }

        removeHash(counter: number): void {
            if (counter === this.counter) {
                this.counter--;
            }
            this.used_hash.delete(counter);
        }

        static GetCounterWithName(
            nameStr: string,
            duplicateNameTrackerMap: Map<string, DuplicateNameTracker>
        ): number {
            let counter = 1;

            if (duplicateNameTrackerMap.has(nameStr)) {
                const duplicateNameTracker = duplicateNameTrackerMap.get(nameStr)!;
                counter = duplicateNameTracker.getHash();
            } else {
                const duplicateNameTracker = new DuplicateNameTracker();
                duplicateNameTracker.counter = counter;
                duplicateNameTracker.used_hash.add(duplicateNameTracker.counter);
                duplicateNameTrackerMap.set(nameStr, duplicateNameTracker);
            }

            return counter;
        }
    }

    export interface KanbanLabel {
        color: string;
        name: string;
        tasks: KanbanTask[];
    }

    export interface KanbanList {
        name: string;
        counter: number;
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