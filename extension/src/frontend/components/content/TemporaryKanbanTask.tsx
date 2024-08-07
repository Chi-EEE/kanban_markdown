import styles from './KanbanList.module.css';

import type { Component, Setter } from 'solid-js';
import { KanbanMarkdown } from "../../types";
import { produce, SetStoreFunction } from 'solid-js/store';

type TemporaryKanbanTaskProps = {
    setState: SetStoreFunction<KanbanMarkdown.State>;

    kanban_list: KanbanMarkdown.KanbanList;

    applyAutoResize: (target: HTMLTextAreaElement) => void;
    setAddButtonVisiblity: Setter<boolean>;
    setCardTextAreaReference: Setter<HTMLTextAreaElement>;
};

export const TemporaryKanbanTask: Component<TemporaryKanbanTaskProps> = (props) => {
    const {
        setState,

        kanban_list,

        applyAutoResize,
        setAddButtonVisiblity,
        setCardTextAreaReference,
    } = props;

    const createTask = (event: FocusEvent) => {
        const target = event.target as HTMLTextAreaElement;
        target.value = target.value.replace(/[\v\n]+/g, '').trim();
        if (target.value !== '') {
            const new_task: KanbanMarkdown.KanbanTask = {
                name: target.value,
                counter: 0,
                checked: false,
                description: '',
                labels: [],
                attachments: [],
                checklist: [],
            };
            // @ts-ignore
            vscode.postMessage({
                commands: [
                    {
                        action: 'create',
                        path: `list["${encodeURI(kanban_list.name)}"].tasks`,
                        value: new_task
                    }
                ]
            });
            setState("kanban_board", "lists", (list, index) => list.name === kanban_list.name, "tasks", produce((tasks: KanbanMarkdown.KanbanTask[]) => {
                tasks.push(new_task);
            }));
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