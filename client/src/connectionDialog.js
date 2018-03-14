import Button from 'material-ui/Button';
import { CircularProgress } from 'material-ui/Progress';
import Dialog from 'material-ui/Dialog';
import { DialogActions } from 'material-ui/Dialog';
import { DialogContent } from 'material-ui/Dialog';
import { DialogContentText } from 'material-ui/Dialog';
import { DialogTitle } from 'material-ui/Dialog';
import TextField from 'material-ui/TextField';
import React from 'react';

/**
 * The Dialog shown when a user attempts to connect to the server.
 * Provides a method for user to change the host address.
 */
class ConnectionDialog extends React.Component {
  /**
   * ConnectDialog constructor.
   * @param {object} props
   */
  constructor(props) {
    super(props);

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

  /**
   * Opens the dialog window.
   */
  open() {
    this.setState({ open:true });
  }

  /**
   * Closes the dialog window.
   */
  close() {
    this.setState({ open:false });
  }

  /**
   * Initiates connection to the host server.
   */
  connect() {
    // debounce multiple connection attempts
    if (this.state.isConnecting) {
      return;
    }

    this.setState({ isConnecting:true });
    this.client.addEventListener('connected', this.onConnect);
    this.client.addEventListener('error', this.onError);
    this.client.connect(this.state.host);
  }

  /**
   * Cancels the attempted server connection.
   */
  cancel() {
    this.client.disconnect();
    this.setState({ isConnecting:false, open:false });
  }

  /**
   * Responds to connection event to reset and close the dialog.
   */
  onConnect() {
    this.setState({ isConnecting:false });
    this.client.removeEventListener('connected', this.onConnect);
    this.client.removeEventListener('error', this.onError);
    this.close();
  }

  /**
   * Responds to a failed connect event.
   */
  onError() {
    this.client.removeEventListener('connected', this.onConnect);
    this.client.removeEventListener('error', this.onError);
    this.setState({ isConnecting:false });
  }

  /**
   * Handles the user updating the host address.
   * @param {Event} event
   */
  handleHostChange(event) {
    this.setState({
      host: event.target.value,
    });
  }

  /**
   * Renders the component to HTML.
   * @return {HTML}
   */
  render() {
    return (
      <Dialog open={this.state.open} onClose={this.close}
        aria-labelledby="form-dialog-title">
        {
          this.state.isConnecting ?
            (<div style={{
              padding: 0,
              margin: 0,
              display: 'flex',
              flexDirection: 'column',
              alignItems: 'center',
              justifyContent: 'center',
            }}>
              <DialogTitle id="form-dialog-title">Connecting...</DialogTitle>
              <CircularProgress/>
            </div>) :
            <DialogTitle id="form-dialog-title">Connect to Server</DialogTitle>
        }
        <DialogContent>
          {
            this.state.isConnecting ? '' :
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
          <Button onClick={this.connect} color="primary"
            disabled={this.state.isConnecting}>
            Connect
          </Button>
        </DialogActions>
      </Dialog>
    );
  }
}

export default ConnectionDialog;
