/* eslint-disable @typescript-eslint/no-explicit-any */
import ReactDOM from 'react-dom';
import { Confirmation } from '../confirmation';
import '../../style/style.css';
import '../../style/modal_style.css';

class Confirmation_Index {
	public static create = (identifer: string, message: string, confirm_data: [type: string, identifier: string]): void => {
		ReactDOM.render(<Confirmation.Make identifier={ identifer } message={ message } confirm_data={ confirm_data }/>,
		document.getElementById('root'));
	};
}

(window as any).Confirmation_Index = Confirmation_Index;