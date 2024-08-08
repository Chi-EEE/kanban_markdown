import styles from './KanbanList.module.css';

import type { Component, Setter } from 'solid-js';
import { KanbanMarkdown } from "../../types";
import { produce, SetStoreFunction } from 'solid-js/store';
import { applyAutoResize } from '../../utils';

type TemporaryKanbanTaskProps = {
    state: KanbanMarkdown.State;
    setState: SetStoreFunction<KanbanMarkdown.State>;

    setAddButtonVisiblity: Setter<boolean>;
    setListTextAreaReference: Setter<HTMLTextAreaElement>;
};

export const TemporaryKanbanList: Component<TemporaryKanbanTaskProps> = (props) => {
    const {
        state,
        setState,

        setAddButtonVisiblity,
        setListTextAreaReference,
    } = props;

    const createTask = (event: FocusEvent) => {
        const target = event.target as HTMLTextAreaElement;
        target.value = target.value.replace(/[\v\n]+/g, '').trim();
        if (target.value !== '') {
            const new_list: KanbanMarkdown.KanbanList = {
                name: target.value,
                counter: KanbanMarkdown.DuplicateNameTracker.GetCounterWithName(target.value, state.kanban_board.list_name_tracker_map),
                checked: false,
                tasks: [],
            };
            setState("kanban_board", "lists", state.kanban_board.lists.length, new_list);
            // @ts-ignore
            vscode.postMessage({
                commands: [
                    {
                        action: 'create',
                        path: `list`,
                        value: new_list
                    }
                ]
            });
        }
        target.value = '';
        setAddButtonVisiblity(true);
    }

    return (
        <a class={styles.kanban_list}>
            <textarea
                class={styles.kanban_list_name}
                ref={(el) => setListTextAreaReference(el)}
                placeholder='Enter list title'
                onBlur={(event) => createTask(event)}
                onKeyPress={(event: KeyboardEvent) => {
                    const target = event.target as HTMLTextAreaElement;
                    if (event.key === 'Enter' && !event.shiftKey) {
                        target.blur();
                    } else {
                        applyAutoResize(target);
                    }
                }}
            >
            </textarea>
        </a>
    );
}