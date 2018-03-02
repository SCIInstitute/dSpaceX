import Button from 'material-ui/Button';
import Dialog from 'material-ui/Dialog';
import { DialogActions } from 'material-ui/Dialog';
import { DialogContent } from 'material-ui/Dialog';
import { DialogContentText } from 'material-ui/Dialog';
import { DialogTitle } from 'material-ui/Dialog';
import TextField from "material-ui/TextField";
import React from 'react';


class ConnectionDialog extends React.Component {
  constructor(props) {
    super(props)

    this.state = {
      open: false,
      host: 'localhost:7681'
    };

    this.open = this.open.bind(this);
    this.close = this.close.bind(this);
    this.handleHostChange = this.handleHostChange.bind(this);
  }

  open() {
    this.setState({ open: true });
  }

  close() {
    this.setState({ open: false });
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
          <DialogTitle id="form-dialog-title">Connect to Server</DialogTitle>
          <DialogContent>
            <DialogContentText>
              Please enter the [host]:[port] of the server.
            </DialogContentText>
            <TextField
              autoFocus
              margin="dense"
              id="host"
              label="Host"
              type="host"
              value={this.state.host}
              onChange={this.handleHostChange}
              fullWidth
            />
          </DialogContent>
          <DialogActions>
            <Button onClick={this.close} color="primary">
              Cancel
            </Button>
            <Button onClick={this.close} color="primary">
              Connect
            </Button>
          </DialogActions>
        </Dialog>
    );
  }
}

export default ConnectionDialog;
