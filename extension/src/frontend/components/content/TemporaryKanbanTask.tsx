import styles from './KanbanList.module.css';

import type { Component, Setter } from 'solid-js';
import { KanbanMarkdown } from "../../types";
import { produce, SetStoreFunction } from 'solid-js/store';
import { applyAutoResize } from '../../utils';

type TemporaryKanbanTaskProps = {
    state: KanbanMarkdown.State;
    setState: SetStoreFunction<KanbanMarkdown.State>;

    kanban_list: KanbanMarkdown.KanbanList;

    setAddButtonVisiblity: Setter<boolean>;
    setCardTextAreaReference: Setter<HTMLTextAreaElement>;
};

export const TemporaryKanbanTask: Component<TemporaryKanbanTaskProps> = (props) => {
    const {
        state,
        setState,

        kanban_list,

        setAddButtonVisiblity,
        setCardTextAreaReference,
    } = props;

    const createTask = (event: FocusEvent) => {
        const target = event.target as HTMLTextAreaElement;
        target.value = target.value.replace(/[\v\n]+/g, '').trim();
        if (target.value !== '') {
            const new_task: KanbanMarkdown.KanbanTask = {
                name: target.value,
                counter: KanbanMarkdown.DuplicateNameTracker.GetCounterWithName(target.value, state.kanban_board.task_name_tracker_map),
                checked: false,
                description: '',
                labels: [],
                attachments: [],
                checklist: [],
            };
            setState("kanban_board", "lists", (list, index) => list.name === kanban_list.name && list.counter === kanban_list.counter, "tasks", produce((tasks: KanbanMarkdown.KanbanTask[]) => {
                tasks.push(new_task);
            }));
            // @ts-ignore
            vscode.postMessage({
                commands: [
                    {
                        action: 'create',
                        path: `list["${encodeURI(kanban_list.name)}"][${kanban_list.counter}].tasks`,
                        value: new_task
                    }
                ]
            });
        }
        target.value = '';
        setAddButtonVisiblity(true);
    }

    return (
        <a class={styles.temp_card}>
            <textarea
                class={styles.temp_card_title}
                ref={(el) => setCardTextAreaReference(el)}
                placeholder='Enter card title'
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