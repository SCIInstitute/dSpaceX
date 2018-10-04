import Button from '@material-ui/core/Button';
import Dialog from '@material-ui/core/Dialog';
import DialogActions from '@material-ui/core/DialogActions';
import DialogContent from '@material-ui/core/DialogContent';
import DialogContentText from '@material-ui/core/DialogContentText';
import DialogTitle from '@material-ui/core/DialogTitle';
import React from 'react';

/**
 * A dialog shown when an unexpected error happens.
 * Provides additional information to the user.
 */
class ErrorDialog extends React.Component {
  /**
   * @param {object} props
   * @constructor
   */
  constructor(props) {
    super(props);
    this.state = {
      open: false,
    };
    this.open = this.open.bind(this);
    this.close = this.close.bind(this);
    this.reportError = this.reportError.bind(this);
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
   * Show the error dialog with the provided error message.
   * @param {string} errorMessage
   */
  reportError(errorMessage) {
    this.setState({
      open: true,
      errorMessage: errorMessage,
    });
  }

  /**
   * Renders the component to HTML.
   * @return {HTML}
   */
  render() {
    return (
      <Dialog open={this.state.open} onClose={this.close}
        aria-labelledby="error-dialog">
        <DialogTitle id="error-dialog">Error</DialogTitle>
        <DialogContent>
          <DialogContentText>
            { this.state.errorMessage }
          </DialogContentText>
        </DialogContent>
        <DialogActions>
          <Button onClick={this.close} color="primary">
            { 'Okay' }
          </Button>
        </DialogActions>
      </Dialog>
    );
  }
};

export default ErrorDialog;
