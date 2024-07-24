import styles from './KanbanList.module.css';

import type { Accessor, Component, Setter } from 'solid-js';
import { KanbanMarkdown } from "../../types";

type TemporaryKanbanTaskProps = {
    applyAutoResize: (target: HTMLTextAreaElement) => void;
    setKanbanListTasks: Setter<KanbanMarkdown.KanbanTask[]>;
    setAddButtonVisiblity: Setter<boolean>;
    setCardTextAreaReference: Setter<HTMLTextAreaElement>;
    getPreviousName: Accessor<string>;
};

export const TemporaryKanbanTask: Component<TemporaryKanbanTaskProps> = (props) => {
    const {
        applyAutoResize,
        setKanbanListTasks,
        setAddButtonVisiblity,
        setCardTextAreaReference,
        getPreviousName,
    } = props;

    return (
        <a class={styles.temp_card}>
            <textarea
                class={styles.temp_card_title}
                ref={(el) => setCardTextAreaReference(el)}
                placeholder='Enter card title'
                onBlur={(event: FocusEvent) => {
                    const target = event.target as HTMLTextAreaElement;
                    target.value = target.value.replace(/[\v\n]+/g, '').trim();
                    if (target.value !== '') {
                        const new_task: KanbanMarkdown.KanbanTask = {
                            name: target.value,
                            checked: false,
                            description: '',
                            labels: [],
                            attachments: [],
                            checklist: [],
                            counter: 0,
                        };
                        // @ts-ignore
                        vscode.postMessage({
                            commands: [
                                {
                                    type: 'create',
                                    path: `list["${encodeURI(getPreviousName())}"].tasks`,
                                    value: new_task
                                }
                            ]
                        });
                        setKanbanListTasks(tasks => [...tasks, new_task]);
                    }
                    target.value = '';
                    setAddButtonVisiblity(true);
                }}
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