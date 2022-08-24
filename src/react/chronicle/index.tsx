/* eslint-disable no-control-regex */
/* eslint-disable @typescript-eslint/no-explicit-any */
import { Modal } from '../modal';
import { Ace } from 'ace-builds';
import ReactDOM from 'react-dom';
import ReactAce from 'react-ace';

class Chronicle_Index {
    public static first_print = true;
    public static point: Ace.Point = { column: 0, row: 4 };
    public static chronicle = new Modal('view', 'chronicle','Chronicle', 'modal', 'none', 'chronicle_bash', 'ace-chronicle', true, 'MoonDweller Automation [Version 1.0.3.10]\nCopyright (©) MoonDweller, LLC. All rights reserved.\n\n', null);
    public static insert_log = (user: string, type: string, input: string): void => {
        (this.chronicle.ace_reference as React.MutableRefObject<ReactAce>).current.editor.getSession().insert(this.point, `[${ new Date().toISOString() }] ${ user }@chronicle[${ type }]:~$ \`${ input }\`\n`);
        this.point.row++;
    };
	public static create = (): void => {
		ReactDOM.render(<this.chronicle.Make/>,
		document.getElementById('root'));
	};
}

(window as any).Chronicle_Index = Chronicle_Index;

export { Chronicle_Index }

Chronicle_Index.create();