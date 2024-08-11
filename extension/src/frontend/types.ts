export namespace KanbanMarkdown {
    export interface State {
        selectedList: KanbanList | undefined;
        selectedTask: KanbanTask | undefined;
        kanban_board: KanbanBoard;
        window_state: {
            taskModalState: boolean;
            scrollTop: number;
            scrollLeft: number;
        }
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
        public counter: number;
        public used_hash: Set<number>;

        constructor(counter: number = 0, used_hash: Set<number> = new Set<number>()) {
            this.counter = counter;
            this.used_hash = used_hash;
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