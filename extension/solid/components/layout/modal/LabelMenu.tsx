import styles from '../TaskModal.module.css';

import { createSignal, For, Show } from "solid-js";
import type { Accessor, Component, Setter } from 'solid-js';
import { KanbanMarkdown } from "../../../types";
import { SetStoreFunction, unwrap } from 'solid-js/store';

type LabelMenuProps = {
    setLabelMenuReference: Setter<HTMLDivElement>;
    kanban_board: KanbanMarkdown.KanbanBoard
    setKanbanBoard: SetStoreFunction<KanbanMarkdown.KanbanBoard>;
    getSelectedList: Accessor<KanbanMarkdown.KanbanList | undefined>;
    getSelectedTask: Accessor<KanbanMarkdown.KanbanTask | undefined>;
    setSelectedTask: Setter<KanbanMarkdown.KanbanTask | undefined>;
};

export const LabelMenu: Component<LabelMenuProps> = (props) => {
    const { setLabelMenuReference, kanban_board, setKanbanBoard, getSelectedList, getSelectedTask, setSelectedTask } = props;

    const [getLabelMenuState, setLabelMenuState] = createSignal<string>("select");

    let modal_new_label_name_input_reference: HTMLInputElement;
    let modal_new_label_color_reference: HTMLInputElement;

    return (
        <div ref={(el) => setLabelMenuReference(el)} class={styles.menu}>
            <Show when={getLabelMenuState() === "select"}>
                <div>
                    <h3>Labels</h3>
                    <div id={styles.modal_label_list}>
                        <For each={kanban_board.labels}>
                            {(kanban_label) => (
                                <div class={styles.modal_label} style={`background-color: ${kanban_label.color}`}
                                    onClick={() => {
                                        const selectedList = getSelectedList();
                                        const selectedTask = getSelectedTask();
                                        // @ts-ignore
                                        vscode.postMessage({
                                            commands: [
                                                {
                                                    action: 'create',
                                                    path: `list["${encodeURI(selectedList.name)}"].tasks["${encodeURI(selectedTask.name)}"][${selectedTask.counter}].labels`,
                                                    value: unwrap(kanban_label),
                                                }
                                            ]
                                        });
                                    }}>
                                    {kanban_label.name}
                                </div>
                            )}
                        </For>
                    </div>
                    <button
                        onClick={() => {
                            setLabelMenuState("create");
                        }}>Create a new label</button>
                </div>
            </Show>
            <Show when={getLabelMenuState() === "create"}>
                <div>
                    <h3>Create Label</h3>
                    <label for="modal-new-label-title">Title</label>
                    <input ref={modal_new_label_name_input_reference} type="text" id="modal-new-label-title" />
                    <label for="modal-new-label-color">Color</label>
                    <input ref={modal_new_label_color_reference} type="color" id="modal-new-label-color" value="#ffffff" />
                    <button id="modal-create-label"
                        onClick={() => {
                            const name = modal_new_label_name_input_reference.value;
                            const color = modal_new_label_color_reference.value;
                            const selectedList = unwrap(getSelectedList());
                            const selectedTask = unwrap(getSelectedTask());
                            // @ts-ignore
                            vscode.postMessage({
                                commands: [
                                    {
                                        action: 'create',
                                        path: `labels`,
                                        value: {
                                            name: name,
                                            color: color,
                                        }
                                    },
                                    {
                                        action: 'create',
                                        path: `list["${encodeURI(selectedList.name)}"].tasks["${encodeURI(selectedTask.name)}"][${selectedTask.counter}].labels`,
                                        value: {
                                            name: name,
                                            color: color,
                                        }
                                    }
                                ]
                            });

                            setKanbanBoard(kanban_board => {
                                kanban_board.labels = kanban_board.labels || [];
                                kanban_board.labels.push({
                                    name: name,
                                    color: color,
                                    tasks: [],
                                });
                                return kanban_board;
                            });
                            setSelectedTask(selectedTask => {
                                selectedTask.labels = selectedTask.labels || [];
                                selectedTask.labels.push({
                                    name: name,
                                    color: color,
                                    tasks: [],
                                });
                                return selectedTask;
                            })
                            setLabelMenuState("select");
                        }}>
                        Create
                    </button>
                    <button id="modal-back-to-label-select"
                        onClick={() => {
                            setLabelMenuState("select");
                        }}>Back</button>
                </div>
            </Show>
        </div>
    );
}
