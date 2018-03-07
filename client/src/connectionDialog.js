import Button from 'material-ui/Button';
import { CircularProgress } from 'material-ui/Progress';
import Dialog from 'material-ui/Dialog';
import { DialogActions } from 'material-ui/Dialog';
import { DialogContent } from 'material-ui/Dialog';
import { DialogContentText } from 'material-ui/Dialog';
import { DialogTitle } from 'material-ui/Dialog';
import TextField from "material-ui/TextField";
import React from 'react';

const ConnectionState = {
  DISCONNECTED: 0,
  CONNECTING: 1,
  CONNECTED: 2,
};

class ConnectionDialog extends React.Component {
  constructor(props) {
    super(props)

    this.state = {
      open: false,
      host: 'localhost:7681',
      isConnecting: false,
    };

    this.open = this.open.bind(this);
    this.close = this.close.bind(this);
    this.connect = this.connect.bind(this);
    this.cancel = this.cancel.bind(this);
    this.handleHostChange = this.handleHostChange.bind(this);
    this.onConnect = this.onConnect.bind(this);
    this.onError = this.onError.bind(this);

    this.client = this.props.client;    
  }

  open() {
    this.setState({ open: true });
  }

  close() {
    this.setState({ open: false });
  }

  connect() {
    // debounce multiple connection attempts
    if (this.state.isConnecting) {
      return;
    }

    this.setState({ isConnecting: true });
    this.client.addEventListener('connected', this.onConnect);
    this.client.addEventListener('error', this.onError);
    this.client.connect(this.state.host);    
  }

  cancel() {    
    this.client.disconnect();
    this.setState({ isConnecting: false, open: false});
  }

  onConnect() {
    this.setState({ isConnecting: false });
    this.client.removeEventListener('connected', this.onConnect);
    this.client.removeEventListener('error', this.onError);
    this.close();
  }

  onError() {
    this.client.removeEventListener('connected', this.onConnect);
    this.client.removeEventListener('error', this.onError);
    this.setState({ isConnecting: false });
  }

  handleHostChange(event) {
    this.setState({
      host: event.target.value,
    });
  }

  render() {
    return (
      <Dialog
          open={this.state.open}
          onClose={this.close}
          aria-labelledby="form-dialog-title"
        >
         {
           this.state.isConnecting ? 
           <div style={{
              padding: 0,
              margin: 0,
              display: 'flex',
              flexDirection: 'column',
              alignItems: 'center',
              justifyContent: 'center'
            }}>
            <DialogTitle id="form-dialog-title">Connecting...</DialogTitle>
            <CircularProgress/>
           </div> :
           <DialogTitle id="form-dialog-title">Connect to Server</DialogTitle>
         }
          <DialogContent>          
            {
              this.state.isConnecting ? ''  :
                <DialogContentText>
                  Please enter the [host]:[port] of the server.
                </DialogContentText>
            }

            <TextField
              autoFocus
              margin="dense"
              id="host"
              label="Host"
              type="host"
              value={this.state.host}
              onChange={this.handleHostChange}
              fullWidth
              disabled={this.state.isConnecting}
            />
          </DialogContent>
          <DialogActions>
            <Button onClick={this.cancel} color="primary">
              Cancel
            </Button>
            <Button onClick={this.connect} color="primary" disabled={this.state.isConnecting}>
              Connect
            </Button>
          </DialogActions>
        </Dialog>
    );
  }
}

export default ConnectionDialog;
